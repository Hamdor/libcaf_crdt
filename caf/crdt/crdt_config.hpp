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

#ifndef CAF_CRDT_CRDT_CONFIG_HPP
#define CAF_CRDT_CRDT_CONFIG_HPP

#include "caf/actor_system_config.hpp"

#include "caf/crdt/detail/replica.hpp"

namespace caf {
namespace crdt {

/// Extended `actor_system_config` to support the CRDT module.
struct crdt_config : public virtual actor_system_config {
  crdt_config() : actor_system_config() {
    load<crdt::replicator>();
  }

  /// Adds crdt `Type` to the module
  template <class Type>
  actor_system_config& add_crdt(const std::string& name) {
    add_message_type<Type>(name);
    add_actor_type<crdt::detail::replica<Type>,
                   const uri&>(name);
    return *this;
  }
};

} // namespace crdt
} // namespace caf

#endif // CAF_CRDT_CRDT_CONFIG_HPP
