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

#include "caf/replication/interfaces/tree.hpp"
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
  std::set<notifyable_t<cmrdt_type>> subscribers_;
  /// Tree relations, parent, childs
  publishable_t<State> tree_parent_;
  std::set<publishable_t<State>> tree_childs_;
};

///
template <class State>
using replica_actor_t = typename subscribable_t<State>::
                          template extend_with<
                            publishable_t<State>,
                            tree_t<State>
                          >;

template <template <class> class Interface, class State>
using injected_stateful_pointer = typename Interface<State>::template
                                    stateful_pointer<replica_state<State>>;

} // namespace <anonymous>

/// Local top level replica actor
template <class State>
typename replica_actor_t<State>::behavior_type
replica_actor(injected_stateful_pointer<replica_actor_t, State> self,
              const std::string& topic) {
  // Initializes new states, which are initialy send to subscribers
  auto init_state = [](const actor& owner, const actor& parent,
                       const State& state) -> State {
    State t{state};
    t.set_owner(owner);
    t.set_parent(parent);
    return t;
  };
  // Init data
  self->state.data_.set_topic(topic);
  return {
    // Handles subscribe requests from actors
    [=](subscribe_atom, const notifyable_t<State>& subscriber) {
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
    [=](unsubscribe_atom, const notifyable_t<State>& subscriber) {
      self->state.subscribers_.erase(subscriber);
    },
    // Buffer and propagate transactions
    [=](publish_atom, typename State::transaction_t& transaction) {
      auto& state = self->state;
      state.data_.apply(transaction);
      auto& sender = self->current_sender();
      // Store state into buffer, if we recieved this through one of our childs
      if (sender != state.tree_parent_)
        state.transaction_buffer_.emplace_back(transaction);
      // Propagate state to our childs
      for (auto& child : state.tree_childs_)
        if (sender != child)
          self->send(child, publish_atom::value, transaction);
      // Propagate transaction to local subscribed actors
      for (auto& subscriber : state.subscribers_)
        if (subscriber != sender)
          self->send(subscriber, notify_atom::value, transaction);
    },
    // Set a parent
    [=](set_parent_atom, const publishable_t<State>& parent) {
      self->state.tree_parent_ = parent;
    },
    // Set childs
    [=](add_child_atom, const publishable_t<State>& child) {
      self->state.tree_childs_.emplace(child);
    },
    // Sends buffer to parent
    // TODO: Make time and loop configurable
    after(std::chrono::seconds(1)) >> [=] {
      auto& state = self->state;
      for (auto& buf : state.transaction_buffer_)
        self->send(state.tree_parent_, publish_atom::value, buf);
      state.transaction_buffer_.clear();
    }
  };
}

} // namespace detail
} // namespace replication
} // namespace caf

#endif // CAF_REPLICATION_DETAIL_REPLICA_ACTOR_HPP
