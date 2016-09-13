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

#ifndef CAF_REPLICATION_DETAIL_ROOT_REPLICA_ACTOR_HPP
#define CAF_REPLICATION_DETAIL_ROOT_REPLICA_ACTOR_HPP

#include <set>
#include <string>
#include <chrono>

#include "caf/atom.hpp"
#include "caf/after.hpp"
#include "caf/typed_actor.hpp"

#include "caf/replication/atom_types.hpp"

#include "caf/replication/interfaces/publish_subscribe.hpp"

namespace caf {
namespace replication {
namespace detail {

namespace {

/// State of top level replica actor
template <class State>
struct root_replica_state {
  /// CmRDT State, this state is propagated to local subscribers
  using delta_type = typename State::internal_t;
  /// Transaction type for CmRDT
  using transaction_t = typename State::transaction_t;
  // Constructor
  root_replica_state() : tree_childs_{} {
    // nop
  }
  /// Internal most recent delta-CRDT state
  delta_type data_;
  /// Childs
  std::set<publishable_type<State>> tree_childs_;
  /// Topic
  std::string topic_;
};

template <class State>
using root_replica_actor = typed_actor<
  reacts_to<message>
>::extend_with<publishable_type<State>>;

} // namespace <anonymous>

/// Local top level replica actor - this actor is to handle the communication
/// between the replicas (CmRDTs) and the Replicator (delta-CRDT). This actor
/// has to translate between both CRDT classes.
template <class State>
typename root_replica_actor<State>::behavior_type
root_replica_actor(typename root_replica_actor<State>::template stateful_pointer<root_replica_state<State>> self,
                   std::string topic) {
  // Init data
  self->state.topic_ = std::move(topic);
  return {
    [=](message& msg) {
      typename State::internal_t delta;
      msg.apply(
        [&](const typename State::internal_t& unpacked) {
          delta = std::move(unpacked);
        }
        // TODO: Add other handler and log errors (should not happen)
      );
      // TODO: Apply delta to our state and generate transactions
      auto& state = self->state;
      auto tmp_delta = state.data_.merge("", delta);
      if (tmp_delta.empty())
        return;
      auto t = tmp_delta.get_cmrdt_transactions(state.topic_,
                                                actor_cast<actor>(self));
      for (auto& child : state.tree_childs_)
        self->send(child, publish_atom::value, t);
    },
    // Buffer and propagate transactions
    [=](publish_atom, typename State::transaction_t& transaction) {
      auto& state = self->state;
      // Send to other childs
      for (auto& child : state.tree_childs_)
        self->send(child, publish_atom::value, transaction);
      auto delta = state.data_.apply(transaction);
      if (delta.empty())
        return;
      self->send(self->system().replicator().actor_handle(),
                 from_local_atom::value, state.topic_, make_message(delta));
    }
  };
}

} // namespace detail
} // namespace replication
} // namespace caf

#endif // CAF_REPLICATION_DETAIL_ROOT_REPLICA_ACTOR_HPP
