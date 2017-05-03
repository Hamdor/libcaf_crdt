/******************************************************************************
 *                       ____    _    _____                                   *
 *                      / ___|  / \  |  ___|    C++                           *
 *                     | |     / _ \ | |_       Actor                         *
 *                     | |___ / ___ \|  _|      Framework                     *
 *                      \____/_/   \_|_|                                      *
 *                                                                            *
 * Copyright (C) 2011 - 2017                                                  *
 * Dominik Charousset <dominik.charousset (at) haw-hamburg.de>                *
 *                                                                            *
 * Distributed under the terms and conditions of the BSD 3-Clause License or  *
 * (at your option) under the terms and conditions of the Boost Software      *
 * License 1.0. See accompanying files LICENSE and LICENSE_ALTERNATIVE.       *
 *                                                                            *
 * If you did not receive a copy of the license files, see                    *
 * http://opensource.org/licenses/BSD-3-Clause and                            *
 * http://www.boost.org/LICENSE_1_0.txt.                                      *
 ******************************************************************************/

#include "caf/crdt/replicator_actor.hpp"

#include "caf/message.hpp"
#include "caf/node_id.hpp"
#include "caf/actor_system_config.hpp"
#include "caf/typed_event_based_actor.hpp"

#include "caf/io/middleman.hpp"

#include "caf/crdt/detail/distribution_layer.hpp"

#include <tuple>
#include <vector>
#include <unordered_map>

namespace caf {
namespace crdt {

namespace {

/// Implementation of replicator actor
class replicator_actor_impl : public replicator_actor::base {
  using interval_res = std::chrono::milliseconds;
public:
  replicator_actor_impl(actor_config& cfg, size_t notify_interval_ms,
                        size_t flush_buffer_interval_ms,
                        size_t state_interval_ms,
                        size_t flush_ids_ms)
      : replicator_actor::base(cfg),
        dist_{this},
        notify_interval_ms_{notify_interval_ms},
        flush_buffer_interval_ms_{flush_buffer_interval_ms},
        state_interval_ms_{state_interval_ms},
        flush_ids_ms_{flush_ids_ms} {
    // nop
  }

  const char* name() const override {
    return "replicator_actor";
  }

protected:
  behavior_type make_behavior() override {
    send(this, tick_buffer_atom::value);
    send(this, tick_state_atom::value);
    send(this, tick_ids_atom::value);
    return {
      // ---
      [&](uri& id, message& msg) {
        if (current_sender()->node() == this->node())
          dist_.publish(id, msg); // Add to send buffer
        auto delegate_to = find_actor(id);
        if (delegate_to)
          delegate(delegate_to, publish_atom::value, std::move(msg));
      },
      [&](uri& id, std::vector<message>& msgs) {
        auto delegate_to = find_actor(id);
        if (delegate_to)
          delegate(delegate_to, publish_atom::value, std::move(msgs));
      },
      [&](tick_state_atom) {
        // All states have to send their state to the replicator
        for (auto& state : states_)
          anon_send(state.second, copy_atom::value);
        delayed_send(this, interval_res(state_interval_ms_),
                     tick_state_atom::value);
      },
      [&](copy_ack_atom, uri& id, message& msg) {
        dist_.publish(std::move(id), std::move(msg));
      },
      [&](tick_ids_atom) {
        dist_.pull_ids();
        delayed_send(this, interval_res(flush_ids_ms_), tick_ids_atom::value);
      },
      [&](tick_buffer_atom) {
        dist_.flush_buffer();
        delayed_send(this, interval_res(flush_buffer_interval_ms_),
                     tick_buffer_atom::value);
      },
      // ---
      [&](new_connection_atom, const node_id& node) {
        dist_.add_new_node(node);
      },
      [&](connection_lost_atom, const node_id& nid) {
        dist_.remove_node(nid);
      },
      [&](get_ids_atom, size_t seen) {
        dist_.get_ids(current_sender()->node(), seen);
      },
      [&](size_t version, std::unordered_set<uri>& ids) {
        dist_.update(current_sender()->node(), version, std::move(ids));
      },
      // --- Subscribe & Unsubscribe
      [&](subscribe_atom, const uri& id) {
        auto delegate_to = find_actor(id);
        if (delegate_to)
          delegate(delegate_to, subscribe_atom::value);
      },
      [&](unsubscribe_atom, const uri& id) {
        auto delegate_to = find_actor(id);
        if (delegate_to)
          delegate(delegate_to, unsubscribe_atom::value);
      },
      // -- Read & Write Consistencies
      [&](read_all_atom, const uri& id) {
        auto delegate_to = find_actor(id);
        if (delegate_to)
          delegate(delegate_to, read_all_atom::value, id,
                   dist_.get_intrested(id));
      },
      [&](read_majority_atom, const uri& id) {
        auto delegate_to = find_actor(id);
        if (delegate_to)
          delegate(delegate_to, read_majority_atom::value, id,
                   dist_.get_intrested(id));
      },
      [&](read_local_atom, const uri& id) {
        auto delegate_to = find_actor(id);
        if (delegate_to)
          delegate(delegate_to, read_local_atom::value);
      },
      [&](write_all_atom, const uri& id, const message& msg) {
        auto delegate_to = find_actor(id);
        if (delegate_to)
          delegate(delegate_to, write_all_atom::value, id,
                   dist_.get_intrested(id), msg);
      },
      [&](write_majority_atom, const uri& id, const message& msg) {
        auto delegate_to = find_actor(id);
        if (delegate_to)
          delegate(delegate_to, write_majority_atom::value, id,
                   dist_.get_intrested(id), msg);
      },
      [&](write_local_atom, const uri& id, const message& msg) {
        auto delegate_to = find_actor(id);
        if (delegate_to)
          delegate(delegate_to, write_local_atom::value, msg);
      }
    };
  }

private:

  actor find_actor(const uri& id) {
    auto iter = states_.find(id);
    if (iter == states_.end()) {
      auto args = make_message(id, notify_interval_ms_);
      auto opt = system().spawn<actor>(id.scheme(), std::move(args));
      if (!opt)
        return {};
      iter = states_.emplace(id, *opt).first;
      dist_.add_id(id);
    }
    return iter->second;
  }

  std::unordered_map<uri, actor> states_; /// Maps from uri to replica<T>
  detail::distribution_layer dist_;       /// Organize dist_ribution of updates
  size_t notify_interval_ms_;             /// Notify interval in milliseconds
  size_t flush_buffer_interval_ms_;       /// Flush interval in milliseconds
  size_t state_interval_ms_;              /// State interval in milliseconds
  size_t flush_ids_ms_;                   /// Replic-ID reconciliation interval
};

} // namespace <anonymous>

replicator_actor make_replicator_actor(actor_system& sys) {
  return sys.spawn<replicator_actor_impl, hidden>(
    sys.config().crdt_notify_interval_ms,
    sys.config().crdt_flush_buffer_interval_ms,
    sys.config().crdt_state_interval_ms,
    sys.config().crdt_ids_interval_ms
  );
}

} // namespace crdt
} // namespace caf
