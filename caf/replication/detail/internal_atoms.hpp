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

#ifndef CAF_REPLICATION_DETAIL_INTERNAL_ATOMS_HPP
#define CAF_REPLICATION_DETAIL_INTERNAL_ATOMS_HPP

#include "caf/atom.hpp"

namespace caf {
namespace replication {
namespace detail {

/// Internal send from a child state to replica
using publish_atom = atom_constant<atom("publish")>;

/// --- TODO: Ab hier alt ==> Aufräumen

/// Used to define a registry key for replicator
using replicator_atom = atom_constant<atom("replicator")>;

/// A new connection to a CAF Node
using new_con = atom_constant<atom("newcon")>;

/// A connection to another CAF Node is lost
using lost_con = atom_constant<atom("lostcon")>;

///
using shutdown = atom_constant<atom("shutdown")>;

/// Send updates to other replicators
using publish_atom = atom_constant<atom("publish")>;

/// Register a CRDT at replicator
using register_atom = atom_constant<atom("regatom")>;

/// Apply a update to local CRDT
using apply_atom = atom_constant<atom("apply")>;

/// Response (delta) of CRDT after apply of update
using apply_ack_atom = atom_constant<atom("applyack")>;

/// Merge two delta updates into one update
using merge_delta = atom_constant<atom("mergedelta")>;

/// Result of delta merge
using merge_delta_ack = atom_constant<atom("deltaack")>;

/// CRDT will push its complete state into a update
using clone_atom = atom_constant<atom("cloneatom")>;

/// Used to get actor handles to replicas
using get_handle_atom = atom_constant<atom("gethandle")>;

/// Setup a CRDT with id, parent,...
using setup_atom = atom_constant<atom("setupatom")>;

} // namespace detail
} // namespace replication
} // namespace caf

#endif // CAF_REPLICATION_DETAIL_INTERNAL_ATOMS_HPP
