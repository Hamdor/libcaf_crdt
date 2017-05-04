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

#define CAF_SUITE vector_clock
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

CAF_TEST_FIXTURE_SCOPE(mv_register_tests, fixture)

/// Test increment and get
CAF_TEST(test_increment_get) {
  auto dummy = system.spawn([](event_based_actor*) {});
  vector_clock clk;
  CAF_CHECK(clk.get(dummy) == 0);
  clk.increment(dummy);
  CAF_CHECK(clk.get(dummy) == 1);
  CAF_CHECK(clk.get(actor{}) == 0);
}

/// Test greater and smaller compares
CAF_TEST(test_compare_greater_smaller) {
  auto dummy1 = system.spawn([](event_based_actor*) {});
  auto dummy2 = system.spawn([](event_based_actor*) {});
  vector_clock clk1;
  vector_clock clk2;
  clk1.increment(dummy1);
  CAF_CHECK(clk1.compare(clk2) == greater);
  CAF_CHECK(clk2.compare(clk1) == smaller);
  clk2.increment(dummy1);
  clk2.increment(dummy2);
  CAF_CHECK(clk1.compare(clk2) == smaller);
  CAF_CHECK(clk2.compare(clk1) == greater);
}

/// test compare equal
CAF_TEST(test_compare_equal) {
  auto dummy1 = system.spawn([](event_based_actor*) {});
  auto dummy2 = system.spawn([](event_based_actor*) {});
  vector_clock clk1;
  vector_clock clk2;
  CAF_CHECK(clk1.compare(clk2) == equal);
  CAF_CHECK(clk2.compare(clk1) == equal);
  clk1.increment(dummy1);
  clk2.increment(dummy1);
  CAF_CHECK(clk1.compare(clk2) == equal);
  CAF_CHECK(clk2.compare(clk1) == equal);
  clk1.increment(dummy2);
  clk2.increment(dummy2);
  CAF_CHECK(clk1.compare(clk2) == equal);
  CAF_CHECK(clk2.compare(clk1) == equal);
  clk1.increment(dummy1);
  CAF_CHECK(clk2.compare(clk1) != equal);
  CAF_CHECK(clk1.compare(clk2) != equal);
}

/// Test compare simultaneous
CAF_TEST(test_simultaneous) {
  auto dummy1 = system.spawn([](event_based_actor*) {});
  auto dummy2 = system.spawn([](event_based_actor*) {});
  vector_clock clk1;
  vector_clock clk2;
  for (int i = 0; i < 2; ++i) {
    clk1.increment(dummy1);
    clk2.increment(dummy2);
    CAF_CHECK(clk1.compare(clk2) == concurrent);
  }
}

CAF_TEST_FIXTURE_SCOPE_END()
