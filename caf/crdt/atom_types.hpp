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

#ifndef CAF_CRDT_ATOM_TYPES_HPP
#define CAF_CRDT_ATOM_TYPES_HPP

#include "caf/atom.hpp"

namespace caf {
namespace crdt {

// -------- Public usable atoms ------------------------------------------------

/// Send to subscribed actors when a replica has changed
using notify_atom = atom_constant<atom("notify")>;

/// Send to replicator to write to K nodes
using read_k_atom = atom_constant<atom("readK")>;

/// Send to replicator to read states from a majority of nodes
using read_majority_atom = atom_constant<atom("readMajor")>;

/// Send to replicator to read states from all nodes
using read_all_atom = atom_constant<atom("readAll")>;

/// Send to replicator to read only local nodes state
using read_local_atom = atom_constant<atom("readLocal")>;

/// Send to replicator to write to K nodes
using write_k_atom = atom_constant<atom("writeK")>;

/// Send to replicator to write to a majority of nodes
using write_majority_atom = atom_constant<atom("writeMajor")>;

/// Send to replicator to write to all nodes
using write_all_atom = atom_constant<atom("writeAll")>;

/// Send to replicator to write to local node
using write_local_atom = atom_constant<atom("writeLocal")>;

/// Send back if write succeed
using write_succeed_atom = atom_constant<atom("wSuccceed")>;

/// Send back if read succeed
using read_succeed_atom = atom_constant<atom("rSucceed")>;

/// Send back if write failed
using write_failed_atom = atom_constant<atom("wFailed")>;

/// Send back if read failed
using read_failed_atom = atom_constant<atom("rFailed")>;

/// Locally delete a replica
using delete_replica = atom_constant<atom("delRepl")>;


// -------- Internal atoms -----------------------------------------------------

/// @private
using replicator_atom = atom_constant<atom("replicator")>;

/// @private
using tick_state_atom = atom_constant<atom("tickState")>;

/// @private
using tick_ids_atom = atom_constant<atom("tickIds")>;

/// @private
using tick_buffer_atom = atom_constant<atom("tickBuf")>;

/// @private
using shutdown_atom = atom_constant<atom("shutdown")>;

/// @private
using copy_atom = atom_constant<atom("copyAtom")>;

/// @private
using copy_ack_atom = atom_constant<atom("copyAck")>;

/// @private
using new_connection_atom = atom_constant<atom("newCon")>;

/// @private
using connection_lost_atom = atom_constant<atom("conLost")>;

/// @private
using get_ids_atom = atom_constant<atom("getIds")>;

/// @private
using timeout_atom = atom_constant<atom("timeout")>;

} // namespace crdt
} // namespace caf

#endif // CAF_CRDT_ATOM_TYPES_HPP
