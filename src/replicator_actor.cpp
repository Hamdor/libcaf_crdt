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

#include "caf/replication/replicator_actor.hpp"

#include "caf/message.hpp"
#include "caf/node_id.hpp"
#include "caf/typed_event_based_actor.hpp"

#include "caf/io/middleman.hpp"

#include "caf/replication/detail/replica_map.hpp"

#include <map>
#include <tuple>
#include <vector>

namespace caf {
namespace replication {

namespace {

/// Implementation of replicator actor
struct replicator_actor_impl : public replicator_actor::base {

  replicator_actor_impl(actor_config& cfg) : replicator_actor::base(cfg) {
    // nop
  }

  virtual ~replicator_actor_impl() = default;

  const char* name() const override {
    return "replicator_actor";
  }

protected:
  behavior_type make_behavior() override {
    send(this, tick_atom::value);
    return {
      [&](const std::string& topic, message& msg) {
        updates_.emplace_back(std::make_tuple(topic, std::move(msg)));
      },
      [&](tick_atom) {
        for(auto& update : updates_) {
          auto& topic = std::get<0>(update);
          auto& payload = std::get<1>(update);
          for (auto& nid : topic_users_.lookup(topic)) {
            auto& repl = replicator_map_[nid];
            anon_send(repl, topic, payload); // TODO: Remove anon_send
          }
        }
        updates_.clear();
        delayed_send(this, std::chrono::milliseconds(250), tick_atom::value);
      },
      [&](new_direct_con, const node_id&) {
        // TODO: Get topics and add to list
      },
      [&](new_indirect_con, const node_id&) {
        // TODO: Get topics and add to list
      },
      [&](con_lost, const node_id& nid) {
        // Remove node id from our base
        topic_users_.remove_node(nid);
      }
    };
  }

private:
  std::vector<std::tuple<std::string, message>> updates_; // Update buffer
  std::map<node_id, actor> replicator_map_; // Map node_ids to replicators
  detail::replica_map topic_users_; // Map topics to subsribers
};

} // namespace <anonymous>

replicator_actor make_replicator_actor(actor_system& sys) {
  return sys.spawn<replicator_actor_impl, hidden>();
}

} // namespace replication
} // namespace caf
