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

#ifndef CAF_REPLICATION_DETAIL_REPLICA_ACTOR_HPP
#define CAF_REPLICATION_DETAIL_REPLICA_ACTOR_HPP

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
struct replica_state {
  /// CmRDT State, this state is propagated to local subscribers
  using cmrdt_type = State;
  /// Transaction type for CmRDT
  using transaction_t = typename cmrdt_type::transaction_t;
  // Constructor
  replica_state() : tree_parent_{unsafe_actor_handle_init}, tree_childs_{} {
    // nop
  }
  /// Internal most current CmRDT state
  cmrdt_type data_;
  /// Buffer for transactions
  std::vector<transaction_t> transaction_buffer_;
  /// Local subscribed actors
  std::set<notifyable_type<cmrdt_type>> subscribers_;
  /// Tree relations, parent, childs
  publishable_type<State> tree_parent_;
  std::set<publishable_type<State>> tree_childs_;
};

///
template <class State>
using replica_actor_type = typename subscribable_type<State>::
                             template extend_with<publishable_type<State>>;

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
                       const State& state) -> State {
    State t{state};
    t.set_owner(owner);
    t.set_parent(parent);
    return t;
  };
  // Init data
  self->state.data_.set_topic(std::move(topic));
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
                          actor_cast<actor>(self), state.data_);
      return std::make_tuple(initial_atom::value, std::move(t));
    },
    // Handle unsubscribe requests from actors
    [=](unsubscribe_atom, const notifyable_type<State>& subscriber) {
      self->state.subscribers_.erase(subscriber);
    },
    // Buffer and propagate transactions
    [=](publish_atom, typename State::transaction_t& transaction) {
      auto& state = self->state;
      state.data_.apply(transaction);
      // Store state into buffer
      state.transaction_buffer_.emplace_back(transaction); // TODO: Collapse function wenn zu groß?
      // Propagate transaction to local subscribed actors
      auto& sender = self->current_sender();
      for (auto& subscriber : state.subscribers_)
        if (subscriber != sender)
          self->send(subscriber, notify_atom::value, transaction);
    },
    // Sends buffer to parent
    // TODO: Make time and loop configurable
    after(std::chrono::seconds(1)) >> [=] {
      auto& state = self->state;
      // TODO: Collaps buffer to single transaction... (if possible)
      for (auto& child : state.tree_childs_)
        for (auto& buf : state.transaction_buffer_)
          self->send(child, publish_atom::value, buf);
    }
  };
}

} // namespace detail
} // namespace replication
} // namespace caf

#endif // CAF_REPLICATION_DETAIL_REPLICA_ACTOR_HPP
