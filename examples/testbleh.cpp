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

#include "caf/all.hpp"

#include "caf/replication/detail/composables/translator.hpp"
#include "caf/replication/detail/composables/subscribable.hpp"
#include "caf/replication/detail/composables/publisher.hpp"

#include "caf/replication/detail/replica_actor.hpp"

#include "caf/replication/crdt/gset.hpp"
#include "caf/replication/crdt/gcounter.hpp"

using namespace caf;
using namespace replication;
using namespace replication::detail;

int main() {
  auto cfg = actor_system_config{};
  actor_system system{cfg};
  // ------
  system.spawn<composed_behavior<subscribable<crdt::gcounter<int>>>>();
  system.spawn<composed_behavior<translator<crdt::gcounter<int>>>>();
  system.spawn<composed_behavior<publisher<crdt::gcounter<int>>>>();
  // ------
  system.spawn<composed_behavior<subscribable<crdt::gset<int>>>>();
  system.spawn<composed_behavior<translator<crdt::gset<int>>>>();
  system.spawn<composed_behavior<publisher<crdt::gset<int>>>>();
  // -------
  system.spawn<root_replica_actor<crdt::gset<int>>>();
  system.spawn<replica_actor<crdt::gset<int>>>();
  // -------
  system.spawn<root_replica_actor<crdt::gcounter<int>>>();
  system.spawn<replica_actor<crdt::gcounter<int>>>();
}
