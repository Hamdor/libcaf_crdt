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

#ifndef CAF_CRDT_TYPES_GMAP_HPP
#define CAF_CRDT_TYPES_GMAP_HPP

#include "caf/crdt/types/base_datatype.hpp"

#include <map>

namespace caf {
namespace crdt {
namespace types {

/// Grow-only Map (GMap)
template <class Key, class Value>
class gmap : public base_datatype {
  /// @private
  gmap(const Key& k, const Value& v) {
    map_.emplace(k, v);
  }

  /// @private
  gmap(std::map<Key, Value>&& map) : map_{std::move(map)} {
    // nop
  }

public:
  using Container = std::map<Key, Value>;
  using const_iterator = typename Container::const_iterator;
  using key_type = typename Container::key_type;
  using mapped_type = typename Container::mapped_type;
  using value_type = typename Container::value_type;

  DECL_CRDT_CTORS(gmap)

  /// Merges two instances of gmap
  /// @param other the second instance to merge
  /// @returns a gmap representing the delta
  gmap merge(const gmap& other) {
    Container delta;
    for (auto& entry : other.map_) {
      auto& key = entry.first;
      auto& value = map_[key];
      if (value < entry.second) {
        value = entry.second;
        delta.emplace(key, entry.second);
      } else if (value > entry.second)
        delta.emplace(key, value);
    }
    return {std::move(delta)};
  }

  /// Set a new value to key. If a key/value pair already exist,
  /// the new value, has to be bigger than the old value to win.
  /// @return `true` if the new value was assigned
  ///         `false` otherwise.
  bool set(const Key& key, const Value& value) {
    auto iter = map_.find(key);
    if (iter == map_.end() || iter->second < value) {
      internal_assign(key, value);
      return true;
    }
    return false;
  }

  /// Returns the value for a specific key
  /// @param key the key to lookup
  /// @returns a valid optional if a entry for the given key exist
  ///          `none` otherwise
  optional<Value> get(const Key& key) const {
    auto iter = map_.find(key);
    if (iter == map_.end())
      return none;
    return iter->second;
  }

  /// @returns a const iterator for a given key
  inline const_iterator find(const Key& key) const { return map_.find(key); }

  /// @returns `true` if the map is empty, `false` otherwise
  inline bool empty() const { return map_.empty(); }

  /// @returns the size of the map
  inline size_t size() const { return map_.size(); }

  /// @private
  template <class Processor>
  friend void serialize(Processor& proc, gmap& x) {
    proc & x.map_;
  }

  /// @returns a const iterator to the ending of the internal map
  inline const_iterator cend() { return map_.cend(); }

  /// @returns a const iterator to the bedinning of the internal map
  inline const_iterator cbegin() { return map_.cbegin(); }

private:
  void internal_assign(const Key& key, const Value& value) {
    map_[key] = value;
    publish(gmap{key, value});
  }

  Container map_;  /// Internal map
};

} // namespace types
} // namespace crdt
} // namespace caf

#endif // CAF_CRDT_TYPES_GMAP_HPP
