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

#ifndef CAF_REPLICATION_INTERFACES_HPP
#define CAF_REPLICATION_INTERFACES_HPP

#include "caf/typed_actor.hpp"

#include "caf/replication/atom_types.hpp"

namespace caf {
namespace replication {

/// Interface definition for actors which work with CRDT States and support
/// notifications.
template <class State>
using notifyable = typed_actor<
  reacts_to<initial_atom, State>,
  reacts_to<notify_atom, typename State::transaction_t>
>;

/// For composables with a tick handler
using tick_t = typed_actor<
  reacts_to<tick_atom>
>;

/// Interface definition for actors which support publish
template <class State>
using publishable = typename typed_actor<
  reacts_to<publish_atom, typename State::transaction_t>
>::template extend_with<tick_t>;

/// Interface definition for actors which support subscribe/unsubscribe
template <class State>
using subscribable_t = typed_actor<
  typename replies_to<
    subscribe_atom,
    notifyable<State>
  >::template with<initial_atom, State>,
  reacts_to<unsubscribe_atom, notifyable<State>>,
  reacts_to<set_parent_atom, publishable<State>>,
  reacts_to<add_child_atom, publishable<State>>
>;

/// Interface for actors, which support translation between CmRDT and
/// CvRDTs
template <class State>
using translator_t = typename typed_actor<reacts_to<message>>::template extend_with<publishable<State>>;

} // namespace replication
} // namespace caf

#endif // CAF_REPLICATION_INTERFACES_HPP
