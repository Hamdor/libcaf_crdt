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

#ifndef CAF_REPLICATION_REPLICATOR_ACTOR_HPP
#define CAF_REPLICATION_REPLICATOR_ACTOR_HPP

#include <string>

#include "caf/fwd.hpp"
#include "caf/typed_actor.hpp"

#include "caf/replication/atom_types.hpp"

namespace caf {
namespace replication {

/// Interface of replicator
using replicator_actor = typed_actor<
  reacts_to<std::string, message>,
  reacts_to<tick_atom>,
  reacts_to<new_direct_con, node_id>,
  reacts_to<new_indirect_con, node_id>,
  reacts_to<con_lost, node_id>
>;

///@relates replicator_actor
replicator_actor make_replicator_actor(actor_system& sys);

} // namespace replication
} // namespace caf

#endif // CAF_REPLICATION_REPLICATOR_ACTOR_HPP
