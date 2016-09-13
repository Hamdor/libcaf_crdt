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

#ifndef CAF_REPLICATION_ATOM_TYPES_HPP
#define CAF_REPLICATION_ATOM_TYPES_HPP

#include "caf/atom.hpp"

namespace caf {
namespace replication {

/// Initial atom is recieved after subscribe to a replica
using initial_atom = atom_constant<atom("init")>;

/// Send to subscribed actors when a replica has changed
using notify_atom = atom_constant<atom("notify")>;

/// Atom used to register the replicator
using replicator_atom = atom_constant<atom("replicator")>;

/// @private
using from_local_atom = atom_constant<atom("fromlocal")>;

/// @private
using from_remote_atom = atom_constant<atom("fromremote")>;

} // namespace replication
} // namespace caf

#endif // CAF_REPLICATION_ATOM_TYPES_HPP
