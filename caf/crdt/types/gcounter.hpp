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

#ifndef CAF_CRDT_TYPES_GCOUNTER_HPP
#define CAF_CRDT_TYPES_GCOUNTER_HPP

#include <tuple>
#include <vector>
#include <unordered_map>

#include "caf/crdt/types/base_datatype.hpp"

// TODO: Add unit test for this type!

namespace caf {
namespace crdt {
namespace types {

/// Implementation of a grow-only counter (GCounter)
template <class T>
class gcounter : public base_datatype {
  /// @private
  gcounter(std::unordered_map<actor, T> map)
    : base_datatype(), map_(std::move(map)) {
    // nop
  }

public:
  DECL_CRDT_CTORS(gcounter)

  /// Increment the counter by value
  /// @param value to increment
  void increment_by(T value) {
    auto key = this->owner();
    value = map_[key] += value;
    std::unordered_map<actor, T> tmp;
    tmp.emplace(key, value);
    publish(gcounter{std::move(tmp)});
  }

  /// Increment the counter by one
  void increment() { increment_by(1); }

  /// Get the count of the counter
  /// @return the count
  inline T count() const {
    auto binary_op = [](T value, const std::pair<actor, T>& p) {
      return value + p.second;
    };
    return std::accumulate(map_.begin(), map_.end(), 0, binary_op);
  }

  /// Merges two CRDT instances
  /// @param other delta-CRDT to merge into this
  /// @returns a delta gcounter<T>
  gcounter<T> merge(const gcounter<T>& other) {
    std::unordered_map<actor, T> delta;
    for (auto& elem : other.map_) {
      auto& key = elem.first;
      auto& value = map_[key];
      if (value < elem.second) {
        value = elem.second;
        delta.emplace(key, elem.second);
      }
    }
    return {std::move(delta)};
  }

  /// @returns `true` if the state is empty
  ///          `false` otherwise
  inline bool empty() const { return map_.empty(); }

  /// @private
  template <class Processor>
  friend void serialize(Processor& proc, gcounter<T>& x) {
    proc & x.map_;
  }

private:
  std::unordered_map<actor, T> map_; /// Map nodes to values
};

} // namespace types
} // namespace crdt
} // namespace caf

#endif // CAF_CRDT_TYPES_GCOUNTER_HPP
