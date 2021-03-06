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

#ifndef CAF_CRDT_VECTOR_CLOCK_HPP
#define CAF_CRDT_VECTOR_CLOCK_HPP

#include "caf/actor.hpp"

#include <unordered_map>

namespace caf {
namespace crdt {

/// Return type for vector clock compares
/// @relates `vector_clock`
enum vector_clock_result {
  greater,
  equal,
  smaller,
  concurrent
};

/// Vector clock implementation for tracking events with vector timestamps
class vector_clock {
  /// Internal map type
  using map_type = std::unordered_map<actor, uint64_t>;
  /// Iternal value type of map
  using value_type = typename map_type::value_type;

  /// @private
  vector_clock(map_type&& map) : map_(std::move(map)) {
    // nop
  }

  /// @priavte
  vector_clock(const map_type::key_type& key, map_type::mapped_type& value) {
    map_.emplace(key, value);
  }

public:
  /// Default constructor
  vector_clock() = default;

  /// Copy constructor
  vector_clock(const vector_clock&) = default;

  /// Increments the slot of given actor
  /// @param slot actor handle for key to increment slot
  /// @param delta specifies if the returned state represents the delta or
  ///        the full clock. Often the full clock is needed inside of CRDTs
  /// @returns vector_clock representing the full clock or delta
  vector_clock increment(const actor& slot, bool delta = false);

  /// Returns the value of given slot
  /// @param slot actor handle of slot
  /// @returns the value for given slot
  size_t get(const actor& slot) const;

  /// Compare two vector clocks and return a value of `vector_clock_result`
  /// @param other `vector_clock` to compare to
  /// @returns `greater` if `other` is greater                 this < other
  ///          `equal`   if `other` is equal to `this`         this == other
  ///          `smaller` if `other` is smaller                 this > other
  ///          `concurrent` if there are concurrent events   this || other
  vector_clock_result compare(const vector_clock& other) const;

  /// Merges to `other` into `this` and returns the delta
  /// @param other `vector_clock` to merge into `this`
  /// @returns vector_clock representing the delta
  vector_clock merge(const vector_clock& other);

  /// Get the count of all slots (number of events in the clock)
  size_t count() const;

  /// @private
  template <class Processor>
  friend void serialize(Processor& proc, vector_clock& x) {
    proc & x.map_;
  }

  /// @private
  friend bool operator< (const vector_clock& lhs, const vector_clock& rhs) {
    return lhs.compare(rhs) != vector_clock_result::concurrent;
  }

private:
  size_t count(const actor& slot) const;

  map_type map_; /// Internal map
};

} // namespace crdt
} // namespace caf

#endif // CAF_CRDT_VECTOR_CLOCK_HPP
