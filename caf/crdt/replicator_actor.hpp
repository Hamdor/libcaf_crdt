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

#ifndef CAF_CRDT_REPLICATOR_ACTOR_HPP
#define CAF_CRDT_REPLICATOR_ACTOR_HPP

#include "caf/fwd.hpp"
#include "caf/node_id.hpp"

#include "caf/typed_actor.hpp"

#include "caf/crdt/uri.hpp"
#include "caf/crdt/atom_types.hpp"

#include <unordered_set>

namespace caf {
namespace crdt {

/// Interface of replicator
using replicator_actor = typed_actor<
  /// Topic message pair, where the message contains updates for a topic
  reacts_to<uri, message>,
  /// Internal tick message to flush updates
  reacts_to<tick_buffers_atom>,
  /// Internal tick message to flush topics
  reacts_to<tick_topics_atom>,
  /// A new connection to a CAF node (node_id) is established
  reacts_to<new_connection_atom, node_id>,
  /// A connection to a CAF node (node_id) is lost
  reacts_to<connection_lost_atom, node_id>,



  /// Return a unordered set of uris to sender
  reacts_to<get_topics_atom, size_t>,
  reacts_to<size_t, std::unordered_set<uri>>,




  /// Subscribes a actor to a replica topic
  reacts_to<subscribe_atom, uri>,
  /// Unsubscribes a actor from a replica topic
  reacts_to<unsubscribe_atom, uri>
>;

///@relates replicator_actor
replicator_actor make_replicator_actor(actor_system& sys);

} // namespace crdt
} // namespace caf

#endif // CAF_CRDT_REPLICATOR_ACTOR_HPP
