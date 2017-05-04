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

#define CAF_SUITE mv_register
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
  mv_register<int> lhs, rhs;
  lhs.set_owner(system.spawn(dummy_actor));
  rhs.set_owner(system.spawn(dummy_actor));
  lhs.set(1);
  CAF_CHECK(lhs.get_set().size() == 1);
  CAF_CHECK(lhs.get() == 1);
  rhs.set(2);
  CAF_CHECK(rhs.get_set().size() == 1);
  CAF_CHECK(rhs.get() == 2);
  lhs.merge(rhs);
  CAF_CHECK(lhs.get_set().size() == 2);
  if (lhs.get_set().size() == 2) {
    auto first  = *lhs.get_set().begin();
    auto second = *(++lhs.get_set().begin());
    CAF_CHECK(std::get<0>(first) == 1);
    CAF_CHECK(std::get<0>(second) == 2);
    CAF_CHECK(std::get<1>(first).compare(std::get<1>(second))
               == vector_clock_result::concurrent);
  }
  rhs.merge(lhs);
  CAF_CHECK(rhs.get_set().size() == 2);
}

CAF_TEST_FIXTURE_SCOPE_END()
