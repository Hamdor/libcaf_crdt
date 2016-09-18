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

#include "caf/after.hpp"
#include "caf/message.hpp"
#include "caf/node_id.hpp"
#include "caf/typed_event_based_actor.hpp"

#include "caf/io/middleman.hpp"

namespace caf {
namespace replication {

namespace {

/// Implementation of replicator actor
struct replicator_actor_impl : public replicator_actor::base {

  replicator_actor_impl(actor_config& cfg)
    : replicator_actor::base(cfg)/*,
      policy_({new update_policy::ring(this)})*/ {
    // nop
  }

  virtual ~replicator_actor_impl() = default;

  const char* name() const override {
    return "replicator_actor";
  }

protected:
  behavior_type make_behavior() override {
    return {
      [&](const std::string& topic, const message& msg) {
        // TODO: We recieved a message from our root_replica, forward to other
        // nodes...
        // for (auto& neighbor : neighbors)
        //   send(neighbor, from_remote_atom::value, topic, msg);
      }
    };
  }

private:
  //std::unique_ptr<update_policy::base_policy> policy_;
};

} // namespace <anonymous>

replicator_actor make_replicator_actor(actor_system& sys) {
  return sys.spawn<replicator_actor_impl, hidden>();
}

} // namespace replication
} // namespace caf
