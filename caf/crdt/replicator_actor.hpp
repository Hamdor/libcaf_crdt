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
using replicator_actor =
  typed_actor<
    /// Replic-ID, message pair, where the message contains updates for a id
    reacts_to<uri, message>,
    /// Replic-ID, vector<message> pair
    reacts_to<uri, std::vector<message>>,
    /// Internal tick message to send complete state, this messages starts the
    /// local collection process of all states.
    reacts_to<tick_state_atom>,
    /// Response to `copy_atom`, the message contains the full state of the replic
    reacts_to<copy_ack_atom, uri, message>,
    /// Internal tick message to flush ids
    reacts_to<tick_ids_atom>,
    /// Internal tick message to flush buffer
    reacts_to<tick_buffer_atom>,
    /// A new connection to a CAF node (node_id) is established
    reacts_to<new_connection_atom, node_id>,
    /// A connection to a CAF node (node_id) is lost
    reacts_to<connection_lost_atom, node_id>,
    /// Return a unordered set of uris to sender
    reacts_to<get_ids_atom, size_t>,
    reacts_to<size_t, std::unordered_set<uri>>,
    /// Subscribes a actor to a replica id
    reacts_to<subscribe_atom, uri>,
    /// Unsubscribes a actor from a replica id
    reacts_to<unsubscribe_atom, uri>,
    /// Reads the value from all nodes
    reacts_to<read_all_atom, uri>,
    /// Reads the value from k nodes
    reacts_to<read_k_atom, size_t, uri>,
    /// Reads the value from a majority of nodes
    reacts_to<read_majority_atom, uri>,
    /// Reads only the local value
    reacts_to<read_local_atom, uri>,
    /// Writes to all nodes
    reacts_to<write_all_atom, uri, message>,
    /// Writes to k nodes
    reacts_to<write_k_atom, size_t, uri, message>,
    /// Writes to a majority of nodes
    reacts_to<write_majority_atom, uri, message>,
    /// Writes only to local node
    reacts_to<write_local_atom, uri, message>
  >;

///@relates replicator_actor
replicator_actor make_replicator_actor(actor_system& sys);

} // namespace crdt
} // namespace caf

#endif // CAF_CRDT_REPLICATOR_ACTOR_HPP
