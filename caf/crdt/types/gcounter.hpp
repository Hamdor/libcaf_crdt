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
struct gcounter : public base_datatype {
  using interface = notifyable<gcounter<T>>;
  using base = typename notifyable<gcounter<T>>::base;
  using behavior_type = typename notifyable<gcounter<T>>::behavior_type;

  gcounter() = default;

  template <class ActorType>
  gcounter(ActorType&& owner, std::string topic)
    : base_datatype(std::move(owner), std::move(topic)) {
    // nop
  }

  ///
  gcounter(std::unordered_map<actor, T> map) : map_(std::move(map)) {
    // nop
  }

  ///
  gcounter(actor key, T value) {
    map_.emplace(key, value);
  }

  ///
  void increment_by(T value) {
    auto key = this->owner();
    value = map_[key] += value;
    publish(gcounter{key, value});
  }

  ///
  void increment() {
    increment_by(1);
  }

  ///
  inline T count() const {
    T result = 0;
    for (auto& elem : map_)
      result += elem.second;
    return result;
  }

  /// Merge function, for this type it is simple
  /// @param other delta-CRDT to merge into this
  /// @returns a delta gcounter<T>
  gcounter<T> merge(const gcounter<T>& other) {
    std::unordered_map<actor, T> delta;
    for (auto& elem : other.map_) {
      auto key = elem.first;
      if (map_[key] < elem.second) {
        map_[key] = elem.second;
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
