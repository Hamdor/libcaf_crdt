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

 #ifndef CAF_REPLICATION_INTERFACES_TREE_HPP
 #define CAF_REPLICATION_INTERFACES_TREE_HPP

#include "caf/typed_actor.hpp"

#include "caf/replication/interfaces/publish_subscribe.hpp"

namespace caf {
namespace replication {

/// Interface to build a tree by providing a set parent function and allow
/// to have childs
template <class State>
using tree_t = typed_actor<
  reacts_to<set_parent_atom, publishable_t<State>>,
  reacts_to<add_child_atom, publishable_t<State>>
>;

} // namespace replication
} // namespace caf

 #endif // CAF_REPLICATION_INTERFACES_TREE_HPP
