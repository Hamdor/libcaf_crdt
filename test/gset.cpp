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

#define CAF_SUITE gset
#include "caf/test/unit_test.hpp"

#include "caf/crdt/all.hpp"

using namespace caf::crdt::types;

void test_merge(const std::set<int>& lhs_, const std::set<int>& rhs_,
                const std::set<int>& assumed_delta_) {
  gset<int> lhs, rhs;
  lhs.subset_insert(lhs_);
  rhs.subset_insert(rhs_);
  auto delta = lhs.merge(rhs);
  for (auto& i : assumed_delta_)
    CAF_CHECK(delta.element_of(i));
  CAF_CHECK(delta.size() == assumed_delta_.size());
}

CAF_TEST(merge) {
  test_merge({1,2,3,4}, {5,6,7,8}, {5,6,7,8});
  test_merge({}, {5,6,7,8}, {5,6,7,8});
  test_merge({1,2,3,4}, {}, {});
  test_merge({1,2,3,4}, {1,2,5}, {5});
}
