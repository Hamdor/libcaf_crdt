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

#ifndef CAF_REPLICATION_REPLICA_ACTOR_HPP
#define CAF_REPLICATION_REPLICA_ACTOR_HPP

#include <set>
#include <string>
#include <chrono>

#include "caf/atom.hpp"
#include "caf/after.hpp"
#include "caf/typed_actor.hpp"

#include "caf/replication/atom_types.hpp"
#include "caf/replication/publish_subscribe.hpp"

namespace caf {
namespace replication {

namespace {

using hierarchical_publish = atom_constant<atom("hierachpub")>;

///
template <class State>
using hierarchical_propagation_type = typed_actor<
  reacts_to<
    hierarchical_publish,
    typename State::internal_type
  >
>;

/// State of top level replica actor
template <class State>
struct replica_state {
  replica_state() : parent_{unsafe_actor_handle_init},
                    child_left_{unsafe_actor_handle_init},
                    child_right_{unsafe_actor_handle_init} {
    // nop
  }

  /// CmRDT State, this state is propagated to local subscribers
  using cmrdt_type = State;
  /// Delta-CRDT, this type is propagated between nodes
  using delta_type = typename State::internal_type;
  /// Internal most current delta-CRDT state
  delta_type delta_;
  /// This type buffers delta-CRDT states, new transactions will generate other
  /// deltas, and triggers a newer version of `delta_`. The delta to the new
  /// current `delta_` will be merged into the `update_buffer_`
  delta_type update_buffer_;
  /// Topic of this top level replica, this is used to identify
  /// replicas on a topic base.
  std::string topic_;
  /// Local subscribed actors
  std::set<notifyable_type<cmrdt_type>> subscribers_;
  /// Sends done to replicator
  size_t loops_;
  // Tree relations, parent, left, right
  hierarchical_propagation_type<State> parent_;
  hierarchical_propagation_type<State> child_left_;
  hierarchical_propagation_type<State> child_right_;
};

///
template <class State>
using replica_actor_type = typename subscribable_type<State>
                             ::template extend_with<publishable_type<State>>
                           ::template extend_with<hierarchical_propagation_type<State>>;

template <template <class> class Interface, class State>
using injected_stateful_pointer = typename Interface<State>::template
                                    stateful_pointer<replica_state<State>>;

} // namespace <anonymous>

/// Local top level replica actor
template <class State>
typename replica_actor_type<State>::behavior_type
replica_actor(injected_stateful_pointer<replica_actor_type, State> self,
              std::string topic) {
  // Initializes new states, which are initialy send to subscribers
  auto init_state = [](const actor& owner, const actor& parent,
                       const std::string& topic,
                       const typename State::internal_type& state) -> State {
    State t;
    t.set_owner(owner);
    t.set_parent(parent);
    t.set_topic(topic);
    t.apply(state.get_cmrdt_transactions(topic));
    return t;
  };
  // Init topic
  self->state.topic_ = std::move(topic);
  return {
    // Handles subscribe requests from actors
    [=](subscribe_atom, const notifyable_type<State>& subscriber) {
      // Check if subscriber is in the same node, if not return a error
      if (self->node() != subscriber.node()) {
        // TODO: Return error if failed
      }
      auto& state = self->state;
      state.subscribers_.emplace(subscriber);
      // Initially send full state
      auto t = init_state(actor_cast<actor>(subscriber),
                          actor_cast<actor>(self), state.topic_, state.delta_);
      return std::make_tuple(initial_atom::value, std::move(t));
    },
    // Handle unsubscribe requests from actors
    [=](unsubscribe_atom, const notifyable_type<State>& subscriber) {
      self->state.subscribers_.erase(subscriber);
    },
    // Buffer and propagate transactions
    [=](publish_atom, typename State::transactions_type& transaction) {
      auto& state = self->state;
      // Apply transaction to delta-CRDT, this will generate a new delta state
      auto to_buffer = state.delta_.apply(transaction);
      // Put the delta state into our update buffer
      state.update_buffer_.unite(to_buffer);
      // Propagate to_buffer to our childs
      if (!state.child_left_.unsafe())
        self->send(state.child_left_, hierarchical_publish::value, to_buffer);
      if (!state.child_right_.unsafe())
        self->send(state.child_right_, hierarchical_publish::value, to_buffer);
      // Propagate transaction to local subscribed actors
      for (auto& subscriber : state.subscribers_)
        if (subscriber != self->current_sender())
          self->send(subscriber, notify_atom::value, transaction);
    },
    // Handle a hierachical publish, this can be recieved from a child or parent
    [=](hierarchical_publish, typename State::internal_type& delta) {
      auto& state = self->state;
      // Apply delta to our state and update buffer
      auto local_delta = state.delta_.merge(delta);
      if (local_delta.empty())
        return; // No change, we already know that state
      auto& sender = self->current_sender();
      if (sender != state.parent_)
        // Only store to update buffer, if we recieved this from a child, then
        // it is our responsibility to forward the state to our parent
        state.update_buffer_.unite(local_delta);
      // Notify childs
      if (sender != state.child_left_ && ! state.child_left_.unsafe())
        self->send(state.child_left_, hierarchical_publish::value, local_delta);
      if (sender != state.child_right_ && ! state.child_right_.unsafe())
        self->send(state.child_right_, hierarchical_publish::value, local_delta);
      // Notify subscribers
      auto transaction = local_delta.get_cmrdt_transactions(state.topic_);
      for (auto& sub : state.subscribers_)
        self->send(sub, notify_atom::value, transaction);
    },
    // Send update_buffer_ or full state to parent
    // TODO: Make time and loop configurable
    after(std::chrono::seconds(1)) >> [=] {
      auto& state = self->state;
      if (state.parent_.unsafe()) // TODO: Remove this if we can build the tree
        return;
      // Check if our parent is the replicator, then we have to send our
      // full state in intervals.
      auto& data = (state.parent_ == self->system().replicator().actor_handle()
                      && ++state.loops_ % 100) ? state.delta_
                                               : state.update_buffer_;
      self->send(state.parent_, hierarchical_publish::value, data);
      state.update_buffer_.clear();
    }
  };
}

} // namespace replication
} // namespace caf

#endif // CAF_REPLICATION_REPLICA_ACTOR_HPP
