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

#include "caf/crdt/vector_clock.hpp"

using namespace caf;
using namespace caf::crdt;

vector_clock vector_clock::increment(const actor& slot) {
  map_[slot]++;
  return {slot, map_[slot]};
}

size_t vector_clock::value_of(const actor& slot) const {
  auto iter = map_.find(slot);
  return iter == map_.end() ? 0 : iter->second;
}

vector_clock vector_clock::merge(const vector_clock& other) {
  map_type delta;
  // Copy entries into delta which are not in `this`
  auto not_in_other = [&](const value_type& e) {
    return other.map_.find(e.first) == other.map_.end();
  };
  std::copy_if(map_.begin(), map_.end(),
               std::inserter(delta, delta.end()), not_in_other);
  // Copy entries which are not in `this` or are less than in `other`
  auto not_in_this_or_max = [&](const value_type& e) {
    auto& key = e.first;
    auto this_iter = map_.find(key);
    if (this_iter == map_.end())
      return true;
    auto other_iter = other.map_.find(key);
    return (this_iter->second < other_iter->second);
  };
  std::copy_if(other.map_.begin(), other.map_.end(),
               std::inserter(delta, delta.end()), not_in_this_or_max);
  // Apply delta to `this`
  for (auto& e : delta) {
    auto key = e.first;
    map_[key] = std::max(map_[key], e.second);
  }
  return {std::move(delta)};
}