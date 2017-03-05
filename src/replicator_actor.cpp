/******************************************************************************
 *                       ____    _    _____                                   *
 *                      / ___|  / \  |  ___|    C++                           *
 *                     | |     / _ \ | |_       Actor                         *
 *                     | |___ / ___ \|  _|      Framework                     *
 *                      \____/_/   \_|_|                                      *
 *                                                                            *
 * Copyright (C) 2011 - 2016                                                  *
 * Dominik Charousset <dominik.charousset (at) haw-hamburg.de>                *
 * Marian Triebe <marian.triebe (at) haw-hamburg.de>                          *
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
struct replicator_actor_impl : public replicator_actor::base {

  replicator_actor_impl(actor_config& cfg)
      : replicator_actor::base(cfg), dist(this) {
    // nop
  }

  const char* name() const override {
    return "replicator_actor";
  }

protected:
  behavior_type make_behavior() override {
    send(this, tick_state_atom::value);
    send(this, tick_topics_atom::value);
    return {
      // ---
      [&](uri& topic, const message& msg) {
        if (current_sender()->node() == this->node())
          dist.publish(topic, msg); // Remote send message
        delegate(find_actor(topic), publish_atom::value, msg);
      },
      [&](tick_state_atom) {
        // All states have to send their state to the replicator
        for (auto& state : states_)
          anon_send(state.second, copy_atom::value);
        delayed_send(this, std::chrono::milliseconds(250),
                     tick_state_atom::value);
      },
      [&](copy_ack_atom, const uri& u, const message& msg) {
        dist.publish(u, msg);
      },
      [&](tick_topics_atom) {
        dist.pull_topics();
        delayed_send(this, std::chrono::milliseconds(250),
                     tick_topics_atom::value);
      },
      // ---
      [&](new_connection_atom, const node_id& node) {
        dist.add_new_node(node);
      },
      [&](connection_lost_atom, const node_id& nid) {
        dist.remove_node(nid);
      },
      [&](get_topics_atom, size_t seen) {
        dist.get_topics(current_sender()->node(), seen);
      },
      [&](size_t version, std::unordered_set<uri>& topics) {
        dist.update(current_sender()->node(), version, std::move(topics));
      },
      // --- Subscribe & Unsubscribe
      [&](subscribe_atom, const uri& u) {
        delegate(find_actor(u), subscribe_atom::value);
      },
      [&](unsubscribe_atom, const uri& u) {
        delegate(find_actor(u), unsubscribe_atom::value);
      }
    };
  }

private:

  actor find_actor(const uri& u) {
    auto iter = states_.find(u);
    if (iter == states_.end()) {
      auto opt = system().spawn<actor>(u.scheme(), make_message(u));
      iter = states_.emplace(u, *opt).first;
    }
    return iter->second;
  }
  ///
  std::unordered_map<uri, actor> states_;
  /// Organize distribution of updates
  detail::distribution_layer dist;
};

} // namespace <anonymous>

replicator_actor make_replicator_actor(actor_system& sys) {
  return sys.spawn<replicator_actor_impl, hidden>();
}

} // namespace crdt
} // namespace caf
