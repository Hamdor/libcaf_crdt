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
      [&](uri& id, const message& msg) {
        if (current_sender()->node() == this->node())
          dist_.publish(id, msg); // Remote send message
        delegate(find_actor(id), publish_atom::value, msg);
      },
      [&](tick_state_atom& atm) {
        // All states have to send their state to the replicator
        for (auto& state : states_)
          anon_send(state.second, copy_atom::value);
        delayed_send(this, interval_res(state_interval_ms_), atm);
      },
      [&](copy_ack_atom, uri& u, message& msg) {
        dist_.publish(std::move(u), std::move(msg));
      },
      [&](tick_ids_atom& atm) {
        dist_.pull_ids();
        delayed_send(this, interval_res(flush_ids_ms_), atm);
      },
      [&](tick_buffer_atom& atm) {
        dist_.flush_buffer();
        delayed_send(this, interval_res(flush_buffer_interval_ms_), atm);
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
      [&](subscribe_atom, const uri& u) {
        delegate(find_actor(u), subscribe_atom::value);
      },
      [&](unsubscribe_atom, const uri& u) {
        delegate(find_actor(u), unsubscribe_atom::value);
      },
      // -- Read & Write Consistencies
      [&](read_all_atom, const uri& u) {
        delegate(find_actor(u), read_all_atom::value, u, dist_.get_intrested(u));
      },
      [&](read_majority_atom, const uri& u) {
        delegate(find_actor(u), read_majority_atom::value, u,
                 dist_.get_intrested(u));
      },
      [&](read_local_atom, const uri& u) {
        delegate(find_actor(u), read_local_atom::value);
      },
      [&](write_all_atom, const uri& u, const message& msg) {
        delegate(find_actor(u), write_all_atom::value, u,
                 dist_.get_intrested(u), msg);
      },
      [&](write_majority_atom, const uri& u, const message& msg) {
        delegate(find_actor(u), write_majority_atom::value, u,
                 dist_.get_intrested(u), msg);
      },
      [&](write_local_atom, const uri& u, const message& msg) {
        delegate(find_actor(u), write_local_atom::value, msg);
      }
    };
  }

private:

  actor find_actor(const uri& u) {
    auto iter = states_.find(u);
    if (iter == states_.end()) {
      auto args = make_message(u, notify_interval_ms_);
      auto opt = system().spawn<actor>(u.scheme(), std::move(args));
      iter = states_.emplace(u, *opt).first;
      dist_.add_id(u);
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
