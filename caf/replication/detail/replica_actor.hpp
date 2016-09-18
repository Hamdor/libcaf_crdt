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

#ifndef CAF_REPLICATION_DETAIL_REPLICA_ACTOR_HPP
#define CAF_REPLICATION_DETAIL_REPLICA_ACTOR_HPP

#include "caf/replication/detail/composables/publisher.hpp"
#include "caf/replication/detail/composables/translator.hpp"
#include "caf/replication/detail/composables/subscribable.hpp"

namespace caf {
namespace replication {
namespace detail {

template <class T> class translator; // TODO: Why need forward declaration here?

/// Type of root replicas
template <class T>
using root_replica_actor = composed_behavior<
  translator<T>,
  subscribable<T>
>;

/// Type of replicas
template <class T>
using replica_actor = composed_behavior<
  publisher<T>,
  subscribable<T>
>;

} // namespace detail
} // namespace replication
} // namespace caf

#endif // CAF_REPLICATION_DETAIL_REPLICA_ACTOR_HPP
