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

#include "caf/node_id.hpp"

#include "caf/crdt/types/base_datatype.hpp"
#include "caf/crdt/types/base_transaction.hpp"

// TODO: Add unit test for this type!

namespace caf {
namespace crdt {
namespace types {

//namespace {

/// A gset support the following mutable operations
enum class gcounter_operations {
  none,
  increment_by,
};

/// Describes a transaction for gcounter<> as CmRDT
template <class T>
struct gcounter_transaction : public base_transaction {
  using operation_t = gcounter_operations;

  gcounter_transaction() = default;

  /// Build a new transaction for gcounter
  /// @param count  the value the counter was incremented by
  /// @param topic of the replica
  /// @param creator  of the transaction
  gcounter_transaction(T count, const std::string& topic, actor creator = {})
    : base_transaction(topic, creator), count_(count),
      op_(operation_t::increment_by) {
    // nop
  }

  /// @returns the value the counter was incremented by
  inline T count() const { return count_; }

  /// @returns the operation done
  inline const operation_t& operation() const { return op_; }

  /// @private
  template <class Processor>
  friend void serialize(Processor& proc, gcounter_transaction<T>& x) {
    proc & x.count_;
    proc & x.op_;
  }

private:
  T count_;
  operation_t op_;
};

namespace delta {

/// This state is hold by top level replica, the CRDT is implementet as
/// delta-CRDT.
/// @private
template <class T>
struct gcounter_impl {
  using operator_t = gcounter_operations;
  using transaction_t = gcounter_transaction<T>;

  gcounter_impl() = default;

  ///
  gcounter_impl(std::unordered_map<node_id, T> map)
      : map_(std::move(map)) {
    // nop
  }

  ///
  gcounter_impl(std::unordered_map<node_id, T> map,
                std::unordered_map<node_id, T> diff)
      : map_(std::move(map)), diff_(std::move(diff)) {
    // nop
  }

  /// Apply transaction from a local subscriber to top level replica
  /// @param history a transaction
  /// @param local node_id
  /// @return a delta-CRDT which represent the delta
  gcounter_impl<T> apply(const transaction_t& history, const node_id& local) {
    std::unordered_map<node_id, T> delta;
    map_[local] += history.count();
    delta.emplace(local, map_[local]);
    return {std::move(delta)};
  }

  /// Merge function, for this type it is simple
  /// @param other delta-CRDT to merge into this
  /// @returns a delta gset<T>
  gcounter_impl<T> merge(gcounter_impl<T>& other) {
    std::unordered_map<node_id, T> delta;
    std::unordered_map<node_id, T> diff;
    auto m = [&](gcounter_impl<T>* lhs, gcounter_impl<T>* rhs, bool d) {
      for (auto& tup : lhs->map_) {
        auto key = tup.first;
        if (rhs->map_[key] < tup.second) {
          // Build difference and store diff
          if (d)
            diff.emplace(key, tup.second - rhs->map_[key]);
          // Overwrite old value
          rhs->map_[key] = tup.second;
          // Store in delta
          delta.emplace(key, tup.second);
        }
      }
    };
    m(&other, this, true);
    m(this, &other, false);
    return {std::move(delta), std::move(diff)};
  }

  /// This is used to convert this delta-CRDT to CmRDT transactions
  /// @param topic  of the replic
  transaction_t get_cmrdt_transactions(const std::string& topic) {
    T total = 0;
    for (auto& e : diff_)
      total += e.second;
    diff_.clear();
    return {total, topic, {}};
  }

  /// @returns `true` if the state is empty
  ///          `false` otherwise
  inline bool empty() const { return map_.empty(); }

  /// Clear the internal set
  inline void clear() { map_.clear(); }

  /// @private
  template <class Processor>
  friend void serialize(Processor& proc, gcounter_impl<T>& x) {
    proc & x.map_;
    proc & x.diff_;
  }

private:
  std::unordered_map<node_id, T> map_; /// Map nodes to values
  std::unordered_map<node_id, T> diff_; /// Stores differences in values which
                                        /// is needed to generate transactions
};

} // namespace delta

namespace cmrdt {

/// Grow only counter as CmRDT.
template <class T>
class gcounter_impl : public base_datatype {
public:

  using operator_t = gcounter_operations;
  /// Mutable operations will trigger this type
  using transaction_t = gcounter_transaction<T>;

  gcounter_impl() : value_{0} {
    // nop
  }

  // --- Delete forbidden operators
  T& operator=(const T&) = delete;
  T operator-=(const T&) = delete;
  T& operator--() = delete;
  T operator--(T) = delete;

  // --- Allowed operators

  /// Increment by operator
  /// @param by number to increment by
  /// @return the new value
  T operator+=(const T& by) {
    if (by > 0) {
      value_ += by;
      publish(transaction_t{by, topic(), owner()});
    }
    return value_;
  }

  /// Pre-Increment operator
  /// @return increment and return new value
  T& operator++() {
    value_ += 1;
    publish(transaction_t{topic(), owner(), operator_t::increment_by, 1});
    return value_;
  }

  /// Post-Increment operator
  /// @return increment and return old value
  T operator++(T) {
    auto cpy = value_;
    operator++();
    return cpy;
  }

  /// @return the current value of gcounter<T>
  T count() const { return value_; }

  /// Apply transaction to local CmRDT type
  /// @param history to apply
  void apply(const transaction_t& history) {
    if (history.operation() == operator_t::increment_by)
      value_ += history.count();
  }

  /// @private
  template <class Processor>
  friend void serialize(Processor& proc, gcounter_impl<T>& x) {
    proc & x.value_;
  }

private:
  T value_; /// Numeric value
};

} // namespace cmrdt

//} // namespace <anonymous>

/// Implementation of a grow-only counter (GCounter)
template <class T>
struct gcounter : public cmrdt::gcounter_impl<T> {
  /// Internal type of gcounter, this implementation is used between
  using internal_t = delta::gcounter_impl<T>;
  /// Internal used for hierarchical propagation
  using transaction_t = typename cmrdt::gcounter_impl<T>::transaction_t;

  using interface = notifyable<gcounter<T>>;
  using base = typename notifyable<gcounter<T>>::base;
  using behavior_type = typename notifyable<gcounter<T>>::behavior_type;

  gcounter() = default;
};

} // namespace types
} // namespace crdt
} // namespace caf

#endif // CAF_CRDT_TYPES_GCOUNTER_HPP
