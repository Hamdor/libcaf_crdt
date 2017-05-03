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

#define CAF_SUITE gcounter
#include "caf/test/unit_test.hpp"

#include "caf/all.hpp"
#include "caf/crdt/all.hpp"

using namespace caf;
using namespace caf::crdt;
using namespace caf::crdt::types;

namespace {

class config : public crdt_config {};

struct fixture {
  fixture() : system{cfg} {
    // nop
  }

  config cfg;
  actor_system system;
};

} // namespace <anonymous>

CAF_TEST_FIXTURE_SCOPE(gcounter_tests, fixture)

CAF_TEST(merge) {
  auto dummy_actor = [](event_based_actor*) {};
  gcounter<int> lhs, rhs;
  lhs.set_owner(system.spawn(dummy_actor));
  rhs.set_owner(system.spawn(dummy_actor));
  lhs.increment();
  CAF_CHECK(lhs.count() == 1);
  rhs.increment();
  CAF_CHECK(rhs.count() == 1);
  auto delta = lhs.merge(rhs);
  CAF_CHECK(lhs.count() == 2);
  CAF_CHECK(rhs.count() == 1);
  CAF_CHECK(delta.count() == 1);
  // --
  delta = rhs.merge(lhs);
  CAF_CHECK(lhs.count() == 2);
  CAF_CHECK(rhs.count() == 2);
  CAF_CHECK(delta.count() == 1);
}

CAF_TEST_FIXTURE_SCOPE_END()
