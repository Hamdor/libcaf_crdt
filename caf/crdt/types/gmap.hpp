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
#include "caf/crdt/types/base_transaction.hpp"

#include <map>

namespace caf {
namespace crdt {
namespace types {

//namespace {

/// Operations supportet by GMap
enum class gmap_operation {
  none,
  assign
};

/// Describes a transaction for a gmap<> as CmRDT
template <class Key, class Value>
struct gmap_transaction : public base_transaction {
  using operation_t = gmap_operation;

  gmap_transaction(std::string topic, operation_t op, Key key, Value value)
      : base_transaction(std::move(topic)),
        op_(std::move(op)) {
    map_.emplace(std::move(key), std::move(value));
  }

  gmap_transaction(std::string topic, operation_t op, std::unordered_map<Key, Value> map)
      : base_transaction(std::move(topic)),
        op_(std::move(op)), map_(std::move(map)) {
    // nop
  }

  gmap_transaction(std::string topic, actor owner, operation_t op,
                   Key key, Value value)
      : base_transaction(std::move(topic), std::move(owner)),
        op_(std::move(op)) {
    map_.emplace(std::move(key), std::move(value));
  }

  gmap_transaction(std::string topic, actor owner, operation_t op,
                   std::unordered_map<Key, Value> map)
      : base_transaction(std::move(topic), std::move(owner)),
        op_(std::move(op)), map_(std::move(map)) {
    // nop
  }

  /// Returns the operations of this transaction
  inline const operation_t& operation() const { return op_; }

  /// Returns the value of this transaction
  inline const std::unordered_map<Key, Value> map() const { return map_; }

  template <class Processor>
  friend void serialize(Processor& proc, gmap_transaction<Key, Value>& x) {
    proc & x.op_;
    proc & x.map_;
  }

  friend std::string to_string(const gmap_transaction&) {
    return {}; // TODO: Implement me
  }

private:
  operation_t op_;
  std::unordered_map<Key, Value> map_;
};

namespace delta {

/// This state is hold by top level replica, the CRDT is implemented as
/// delta-CRDT
/// @private
template <class Key, class Value>
struct gmap_impl {
  using operator_t = gmap_operation;
  using transaction_t = gmap_transaction<Key, Value>;

  /// Default constructor
  gmap_impl() = default;

  gmap_impl(std::unordered_map<Key, Value> map) : map_(std::move(map)) {
    // nop
  }

  /// Apply transaction from local subscriber to top level replica
  /// @param history a transaction
  /// @return a delta-CRDT which represent the delta
  gmap_impl<Key, Value> apply(const transaction_t& history, const node_id&) {
    std::unordered_map<Key, Value> delta;
    if (history.operation() == operator_t::assign) {
      for (auto& pair : history.map()) {
        auto& entry = map_[pair.first];
        if (entry < pair.second) {
          entry = pair.second;
          delta[pair.first] = pair.second;
        }
      }
    }
    return {std::move(delta)};
  }

  /// Merge function for GMap, if we have two values for the same key
  /// the bigger value wins.
  gmap_impl<Key, Value> merge(const std::string&,
                              const gmap_impl<Key, Value>& other) {
    std::unordered_map<Key, Value> delta;
    for (auto& elem : other.map_) {
      auto& key   = elem.first;
      auto& value = elem.second;
      if (map_[key] < value) {
        map_[key]  = value;
        delta[key] = value;
      }
    }
    return {std::move(delta)};
  }

  /// This is used to convert this delta-CRDT to CmRDT transactions
  transaction_t get_cmrdt_transactions(const std::string& topic) const {
    return {topic, operator_t::assign, map_};
  }

  /// @returns `true` if the state is empty
  ///          `false` otherwise
  inline bool empty() const { return map_.empty(); }

  /// Clear the internal map
  inline void clear() { map_.clear(); }

  /// Get reference to internal map
  inline const std::unordered_map<Key, Value>& map() const { return map_; }

  /// @private
  template <class Processor>
  friend void serialize(Processor& proc, gmap_impl<Key, Value>& x) {
    proc & x.map_;
  }

private:
  std::unordered_map<Key, Value> map_;
};

} // namespace delta

namespace cmrdt {

/// Grow only map as CmRDT.
// TODO: http://en.cppreference.com/w/cpp/container/unordered_map alles impln
template <class Key, class Value>
class gmap_impl : public base_datatype {
  using iterator = typename std::unordered_map<Key, Value>::iterator;
  using const_iterator = typename std::unordered_map<Key, Value>::const_iterator;

public:
  using key_type = Key;
  using mapped_type = Value;
  using value_type = std::pair<const Key, Value>;
  // -----
  using operator_t = gmap_operation;
  /// Mutable operations will trigger this type
  using transaction_t = gmap_transaction<Key, Value>;

  gmap_impl() = default;
  gmap_impl(const gmap_impl&) = default;

  /*gmap_impl& operator=(gmap_impl&& other) {
    base_datatype::operator=(std::move(other));
    if (set_.size())
      publish(transaction_t{topic(), owner(), operator_t::assign, map_});
    return *this;
  }*/

  /// Get the value of key
  const Value& at(const Key& key) {
    return map_[key];
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

  /// @returns a reference to internal map
  inline const std::unordered_map<Key, Value> map() const { return map_; }

  //

  inline Value& operator[](const Key& key) { return map_[key]; }

  inline Value& operator[](Key&& key) { return map_[std::move(key)]; }

  inline iterator begin() { return map_.begin(); }
  inline const_iterator begin() const { return map_.begin(); }
  inline const_iterator cbegin() const { return map_.cbegin(); }

  inline iterator end() { return map_.end(); }
  inline const_iterator end() const { return map_.end(); }
  inline const_iterator cend() const { return map_.end(); }

  ///
  inline iterator find(const Key& key) { return map_.find(key); }

  ///
  inline const_iterator find(const Key& key) const { return map_.find(key); }

private:
  void internal_assign(const Key& key, const Value& value) {
    map_[key] = value;
    publish(transaction_t{topic(), owner(), operator_t::assign, key, value});
  }

  std::unordered_map<Key, Value> map_;
};

} // namespace cmrdt

//} // namespace <anonymous>

/// Implementation of a grow-only map (GMap)
template <class Key, class Value>
struct gmap : public cmrdt::gmap_impl<Key, Value> {
  using key_type = Key;
  using value_type = Value;
  /// Internal type of gmap
  using internal_t = delta::gmap_impl<Key, Value>;
};

} // namespace types
} // namespace crdt
} // namespace caf

#endif // CAF_CRDT_TYPES_GMAP_HPP
