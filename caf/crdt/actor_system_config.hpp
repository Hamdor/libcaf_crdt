/******************************************************************************
 *                       ____    _    _____                                   *
 *                      / ___|  / \  |  ___|    C++                           *
 *                     | |     / _ \ | |_       Actor                         *
 *                     | |___ / ___ \|  _|      Framework                     *
 *                      \____/_/   \_|_|                                      *
 *                                                                            *
 * Copyright (C) 2011 - 2016                                                  *
 * Dominik Charousset <dominik.charousset (at) haw-hamburg.de>                *
 * Marian Triebe <marian.triebe (at) haw-hamburg.de>
 *                                                                            *
 * Distributed under the terms and conditions of the BSD 3-Clause License or  *
 * (at your option) under the terms and conditions of the Boost Software      *
 * License 1.0. See accompanying files LICENSE and LICENSE_ALTERNATIVE.       *
 *                                                                            *
 * If you did not receive a copy of the license files, see                    *
 * http://opensource.org/licenses/BSD-3-Clause and                            *
 * http://www.boost.org/LICENSE_1_0.txt.                                      *
 ******************************************************************************/

#ifndef CAF_CRDT_ACTOR_SYSTEM_CONFIG_HPP
#define CAF_CRDT_ACTOR_SYSTEM_CONFIG_HPP

#include "caf/actor_system_config.hpp"

#include "caf/crdt/detail/replica.hpp"

namespace caf {
namespace crdt {

/// Extended `actor_system_config` to support the replication module.
struct replicator_config : public virtual caf::actor_system_config {

  /// Adds replica `Type` to the replicator (if loaded).
  template <class Type>
  actor_system_config& add_replica_type(const std::string& name) {
    add_message_type<Type>(name);
    add_message_type<typename Type::internal_t>(name+"::internal");
    add_actor_type<crdt::detail::replica<Type>,
                   const std::string&>(name);
    return *this;
  }
};

} // namespace crdt
} // namespace caf

#endif // CAF_CRDT_ACTOR_SYSTEM_CONFIG_HPP
