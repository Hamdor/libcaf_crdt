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

#ifndef CAF_CRDT_TYPES_GMAP_HPP
#define CAF_CRDT_TYPES_GMAP_HPP

#include "caf/crdt/types/base_datatype.hpp"

#include <map>

namespace caf {
namespace crdt {
namespace types {

/// Grow-only Map (GMap)
template <class Key, class Value>
struct gmap : public base_datatype {

  using Container = std::map<Key, Value>;

  using const_iterator = typename Container::const_iterator;
  using key_type = typename Container::key_type;
  using mapped_type = typename Container::mapped_type;
  using value_type = typename Container::value_type;

  /// @private
  gmap() = default;

  ///
  template <class ActorType>
  gmap(ActorType&& owner, std::string topic)
    : base_datatype(std::forward<ActorType>(owner), std::move(topic)) {
    // nop
  }

private:

  /// @private
  gmap(const Key& k, const Value& v) {
    map_.emplace(k, v);
  }

public:

  gmap merge(const gmap& other) {
    Container delta;
    for (auto& entry : other.map_) {
      auto& key = entry.first;
      auto& value = entry.second;
      if (map_[key] < value) {
        map_[key] = value;
        delta.emplace(key, value);
      } else if (map_[key] > value)
        delta.emplace(key, map_[key]);
    }
    return {std::move(delta)};
  }

  /// Assign a new value to key. If a key/value pair already exist,
  /// the new value, has to be bigger than the old value to win.
  /// @return `true` if the new value was assigned
  ///         `false` otherwise.
  bool assign(const Key& key, const Value& value) {
    auto iter = map_.find(key);
    if (iter == map_.end() || iter->second < value) {
      internal_assign(key, value);
      return true;
    }
    return false;
  }

  ///
  optional<Value> get(const Key& key) const {
    auto iter = map_.find(key);
    if (iter == map_.end())
      return empty;
    return iter->second;
  }

  ///
  inline const_iterator find(const Key& key) const { return map_.find(key); }

  ///
  inline bool empty() const { return map_.empty(); }

  ///
  inline size_t size() const { return map_.size(); }

  template <class Processor>
  friend void serialize(Processor& proc, gmap& x) {
    proc & x.map_;
  }

private:
  void internal_assign(const Key& key, const Value& value) {
    map_[key] = value;
    publish(gmap{key, value});
  }

  Container map_;  /// Internal map

public:
  ///
  inline auto cend() -> decltype(map_.cend()) { return map_.cend(); }

  ///
  inline auto cbegin() -> decltype(map_.cbegin()) { return map_.cbegin(); }
};

} // namespace types
} // namespace crdt
} // namespace caf

#endif // CAF_CRDT_TYPES_GMAP_HPP
