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

// -------- Public usable atoms ------------------------------------------------

/// Send to subscribed actors when a replica has changed
using notify_atom = atom_constant<atom("notify")>;

/// Send to replicator to read states from a majority of nodes
using read_majority_atom = atom_constant<atom("readmajor")>;

/// Send to replicator to read states from all nodes
using read_all_atom = atom_constant<atom("readall")>;

/// Send to replicator to read only local nodes state
using read_local_atom = atom_constant<atom("readlocal")>;

/// Send to replicator to write to a majority of nodes
using write_majority_atom = atom_constant<atom("writemajor")>;

/// Send to replicator to write to all nodes
using write_all_atom = atom_constant<atom("writeall")>;

/// Send to replicator to write to local node
using write_local_atom = atom_constant<atom("writelocal")>;

/// Send back if write succeed
using write_succeed_atom = atom_constant<atom("wsuccceed")>;

/// Send back if read succeed
using read_succeed_atom = atom_constant<atom("rsucceed")>;

/// Send back if write failed
using write_failed_atom = atom_constant<atom("wfailed")>;

/// Send back if read failed
using read_failed_atom = atom_constant<atom("rfailed")>;

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
using new_connection_atom = atom_constant<atom("newcon")>;

/// @private
using connection_lost_atom = atom_constant<atom("conlost")>;

/// @private
using get_ids_atom = atom_constant<atom("getids")>;

/// @private
using delete_id_atom = atom_constant<atom("delid")>;

} // namespace crdt
} // namespace caf

#endif // CAF_CRDT_ATOM_TYPES_HPP
