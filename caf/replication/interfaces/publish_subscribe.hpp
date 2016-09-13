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

#ifndef CAF_REPLICATION_INTERFACES_PUBLISH_SUBSCRIBE_HPP
#define CAF_REPLICATION_INTERFACES_PUBLISH_SUBSCRIBE_HPP

#include "caf/replication/interfaces/notifyable.hpp"

namespace caf {
namespace replication {

/// Interface definition for actors which support subscribe/unsubscribe
template <class State>
using subscribable_type = typed_actor<
  typename replies_to<
    subscribe_atom,
    notifyable_type<State>
  >::template with<initial_atom, State>,
  reacts_to<unsubscribe_atom, notifyable_type<State>>>;

/// Interface definition for actors which support publish
template <class State>
using publishable_type = typed_actor<
  reacts_to<publish_atom, typename State::transaction_t>
>;

} // namespace replication
} // namespace caf

#endif // CAF_REPLICATION_INTERFACES_PUBLISH_SUBSCRIBE_HPP
