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

#ifndef CAF_REPLICATION_CRDT_GMAP_HPP
#define CAF_REPLICATION_CRDT_GMAP_HPP

#include "caf/replication/crdt/base_datatype.hpp"
#include "caf/replication/crdt/base_transaction.hpp"

#include <map>

namespace caf {
namespace replication {
namespace crdt {

namespace {

/// Operations supportet by GMap
enum class gmap_operation {
  none,
  assign
};

/// Describes a transaction for a gmap<> as CmRDT
template <class Key, class Value>
struct gmap_transaction : public base_transaction {
  using operation_t = gmap_operation;

  /// Construct a new transaction
  gmap_transaction(std::string topic, actor owner, operation_t op,
                   Key key, Value value)
      : base_transaction(std::move(topic), std::move(owner)),
        op_(std::move(op)) {
    map_.emplace(std::move(key), std::move(value));
  }

  gmap_transaction(std::string topic, actor owner, operation_t op,
                   std::map<Key, Value> map)
      : base_transaction(std::move(topic), std::move(owner)),
        op_(std::move(op)), map_(std::move(map)) {
    // nop
  }

  virtual ~gmap_transaction() = default;

  /// Returns the operations of this transaction
  inline const operation_t& operation() const { return op_; }

  /// Returns the value of this transaction
  inline const std::map<Key, Value> map() const { return map_; }

  template <class Processor>
  friend void serialize(Processor& proc, gmap_transaction<Key, Value>& x) {
    proc & x.op_;
    proc & x.map_;
  }

private:
  operation_t op_;
  std::map<Key, Value> map_;
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

  gmap_impl(std::map<Key, Value> map) : map_(std::move(map)) {
    // nop
  }

  /// Apply transaction from local subscriber to top level replica
  /// @param history a transaction
  /// @return a delta-CRDT which represent the delta
  gmap_impl<Key, Value> apply(const transaction_t& history) {
    std::map<Key, Value> delta;
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
    std::map<Key, Value> delta;
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
  /// @param topic for this transaction
  /// @param creator these transactions where done
  transaction_t get_cmrdt_transactions(const std::string& topic,
                                       const actor& creator) const {
    return {topic, creator, operator_t::assign, map_};
  }

  /// @returns `true` if the state is empty
  ///          `false` otherwise
  inline bool empty() const { return map_.empty(); }

  /// Clear the internal map
  inline void clear() { map_.clear(); }

  /// @private
  template <class Processor>
  friend void serialize(Processor& proc, gmap_impl<Key, Value>& x) {
    proc & x.map_;
  }

private:
  std::map<Key, Value> map_;
};

} // namespace delta

namespace cmrdt {

/// Grow only map as CmRDT.
template <class Key, class Value>
class gmap_impl : public base_datatype {
  using iterator = typename std::map<Key, Value>::iterator;

public:
  using key_type = Key;
  using mapped_type = Value;
  using value_type = std::pair<const Key, Value>;
  // -----
  using operator_t = gmap_operation;
  /// Mutable operations will trigger this type
  using transaction_t = gmap_transaction<Key, Value>;

  /// Get the value of key
  const Value& at(const Key& key) {
    return map_[key]; // TODO: Generate update?
  }

  /// Assign a new value to key. If a key/value pair already exist,
  /// the new value, has to be bigger than the old value to win.
  void assign(const Key& key, const Value& value) {
    auto iter = map_.find(key);
    if (iter == map_.end() || iter->second < value)
      internal_assign(key, value);
  }

private:
  void internal_assign(const Key& key, const Value& value) {
    map_[key] = value;
    publish(transaction_t{topic(), owner(), operator_t::assign, key, value});
  }

  std::map<Key, Value> map_;
};

} // namespace cmrdt

} // namespace <anonymous>

/// Implementation of a grow-only map (GMap)
template <class Key, class Value>
struct gmap : public cmrdt::gmap_impl<Key, Value> {
  /// Internal type of gmap
  using internal_t = delta::gmap_impl<Key, Value>;
  /// Internal used for hierachical propagation
  using transaction_t = typename cmrdt::gmap_impl<Key, Value>::transaction_t;
};

} // namespace crdt
} // namespace replication
} // namespace caf

#endif // CAF_REPLICATION_CRDT_GMAP_HPP
