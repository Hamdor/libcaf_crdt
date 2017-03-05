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

#ifndef CAF_CRDT_ATOM_TYPES_HPP
#define CAF_CRDT_ATOM_TYPES_HPP

#include "caf/atom.hpp"

namespace caf {
namespace crdt {

/// Initial atom is recieved after subscribe to a replica
using initial_atom = atom_constant<atom("init")>;

/// Send to subscribed actors when a replica has changed
using notify_atom = atom_constant<atom("notify")>;

// -------- Internal atoms -----------------------------------------------------

/// @private
using replicator_atom = atom_constant<atom("replicator")>;

/// @private
using tick_state_atom = atom_constant<atom("fstate")>;

/// @private
using tick_topics_atom = atom_constant<atom("ftopics")>;

/// @private
using shutdown_atom = atom_constant<atom("shutdown")>;

// -------- Replicator communication atoms -------------------------------------

/// @private
using copy_atom = atom_constant<atom("copyatom")>;

/// @private
using copy_ack_atom = atom_constant<atom("copyack")>;

/// @private
using new_state = atom_constant<atom("newstate")>;

/// @private
using new_connection_atom = atom_constant<atom("newcon")>;

/// @private
using connection_lost_atom = atom_constant<atom("conlost")>;

/// @private
using get_topics_atom = atom_constant<atom("gettopics")>;

/// @private
using add_topic_atom = atom_constant<atom("addtopic")>;

/// @private
using remove_topic_atom = atom_constant<atom("remtopic")>;

/// @private
using update_topics_atom = atom_constant<atom("updatetopi")>;

/// @private
using create_replica_atom = atom_constant<atom("makerepl")>;

} // namespace crdt
} // namespace caf

#endif // CAF_CRDT_ATOM_TYPES_HPP
