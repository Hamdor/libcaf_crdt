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

#ifndef CAF_CRDT_TYPES_GSET_HPP
#define CAF_CRDT_TYPES_GSET_HPP

#include <set>
#include <algorithm>

#include "caf/node_id.hpp"

#include "caf/detail/comparable.hpp"

#include "caf/crdt/types/base_datatype.hpp"
#include "caf/crdt/types/base_transaction.hpp"

namespace caf {
namespace crdt {
namespace types {

//namespace {

/// A gset support the following mutable operations
enum class gset_operations {
  none,
  insertion
};

/// Describes a transation for gset<> as CmRDT
template <class T>
struct gset_transaction : public base_transaction {
  using operation_t = gset_operations;

  /// Construct a new transaction
  gset_transaction(std::string topic, operation_t operation,
                   std::set<T> values)
    : base_transaction(std::move(topic)),
      op_(std::move(operation)),
      values_(std::move(values)) {
    // nop
  }

  /// Construct a new transaction
  gset_transaction(std::string topic, actor owner, operation_t operation,
                   std::set<T> values)
    : base_transaction(std::move(topic), std::move(owner)),
      op_(std::move(operation)),
      values_(std::move(values)) {
    // nop
  }

  /// Returns the operation of this transaction
  inline const operation_t& operation() const { return op_; }

  /// Returns the value of this transaction
  inline const std::set<T>& values() const { return values_; }

  /// @private
  template <class Processor>
  friend void serialize(Processor& proc, gset_transaction<T>& x) {
    proc & x.op_;
    proc & x.values_;
  }

  friend std::string to_string(const gset_transaction<T>& e) {
    std::stringstream ss;
    ss << e.values_.size();
    ss << (e.op_ == operation_t::insertion ? "insert" : "none");
    return ss.str();
  }

private:
  operation_t op_;
  std::set<T> values_;
};

namespace delta {

/// This state is hold by top level replica, the CRDT is implemented as
/// delta-CRDT.
/// @private
template <class T>
struct gset_impl {
  using operator_t = gset_operations;
  using transaction_t = gset_transaction<T>;

  /// Default constructor
  gset_impl() = default;

  gset_impl(std::set<T> delta) : set_(std::move(delta)) {
    // nop
  }

  /// Apply transaction from a local subscriber to top level replica
  /// @param history a transaction
  /// @return a delta-CRDT which represent the delta
  gset_impl<T> apply(const transaction_t& history, const node_id&) {
    std::set<T> delta;
    if (history.operation() != operator_t::insertion)
      return {std::move(delta)};
    for (auto& elem : history.values())
      if (internal_emplace(elem))
        delta.emplace(elem);
    return {std::move(delta)};
  }

  /// Merge function, for this type it is simple
  /// @param other delta-CRDT to merge into this
  /// @returns a delta gset<T>
  gset_impl<T> merge(const gset_impl<T>& other) {
    std::set<T> delta;
    for (auto& elem : other.set_)
      if (internal_emplace(elem))
        delta.emplace(elem);
    return {std::move(delta)};
  }

  /// This is used to convert this delta-CRDT to CmRDT transactions
  transaction_t get_cmrdt_transactions(const std::string& topic) const {
    // For this type its simple again, we just have to copy the set
    return {topic, operator_t::insertion, set_};
  }

  /// @returns `true` if the state is empty
  ///          `false` otherwise
  inline bool empty() const { return set_.empty(); }

  /// Clear the internal set
  inline void clear() { set_.clear(); }

  /// @private
  template <class Processor>
  friend void serialize(Processor& proc, gset_impl<T>& x) {
    proc & x.set_;
  }

private:
  /// @private
  inline bool internal_emplace(const T& elem) {
    return std::get<1>(set_.emplace(elem));
  }
  std::set<T> set_;
};

} // namespace delta

namespace cmrdt {

/// Grow only set as CmRDT.
template <class T>
class gset_impl : public base_datatype {
public:

  using operator_t = gset_operations;
  /// Mutable operations will trigger this type
  using transaction_t = gset_transaction<T>;

  gset_impl(actor owner, actor parent, std::string topic)
    : base_datatype(std::move(owner), std::move(parent), std::move(topic)) {
    // nop
  }

  gset_impl() = default;
  gset_impl(const gset_impl&) = default;

  gset_impl(std::set<T> set) : set_(std::move(set)) {
    // nop
  }

  /// Move assignment, if operations where done, before the state was
  /// initialized we have to send all done operations to other replicas.
  gset_impl& operator=(gset_impl&& other) {
    base_datatype::operator=(std::move(other));
    if (!size())
      set_ = std::move(other.set_); // just move
    else {
      // merge both and publish our operations...
      publish(transaction_t{topic(), owner(), operator_t::insertion, set_});
      for (auto& e : other.set_)
        set_.emplace(std::move(e));
    }
    return *this;
  }

  /// Insert a element into this gset
  /// @param elem to insert
  /// @returns a transaction
  transaction_t insert(const T& elem) {
    auto b = internal_emplace(elem);
    auto result = transaction_t{topic(), owner(), b ? operator_t::insertion
                                                  : operator_t::none, {elem}};
    publish(result);
    return result;
  }

  /// Add a set of elements into gset
  /// @param elems to insert
  /// @returns a set of operations done to the gset
  transaction_t subset_insert(const std::set<T>& elems) {
    std::set<T> insertions;
    for (auto& elem : elems) {
      if (internal_emplace(elem))
        insertions.emplace(elem);
    }
    auto result = transaction_t{topic(), owner(), operator_t::insertion,
                                std::move(insertions)};
    publish(result);
    return result;
  }

  /// Checks if `elem` is in the set
  /// @param elem to check
  bool element_of(const T& elem) const {
    return set_.find(elem) != set_.end();
  }

  /// Checks if `other` includes `other`.
  /// @param other set of elements
  bool is_subset_of(const std::set<T>& other) const {
    return std::includes(other.begin(), other.end(), set_.begin(), other.end());
  }

  /// Checks if `this` includes `other`.
  /// @param other set of elements
  bool is_superset_of(const std::set<T>& other) const {
    return std::includes(set_.begin(), set_.end(), other.begin(), other.end());
  }

  /// Checks if `other` and `this` are equal.
  bool equal(const std::set<T>& other) const {
    return set_ == other;
  }

  /// Apply transaction to local CmRDT type
  /// @param history to apply
  void apply(const transaction_t& history) {
    if (history.operation() == operator_t::none)
      return;
    for (auto& transaction : history.values())
      set_.emplace(transaction);
  }

  /// @returns the number of elements in the set
  size_t size() const { return set_.size(); }

  /// Immutable access to underlying set
  const std::set<T>& get_immutable() const { return set_; }

  /// @private
  template <class Processor>
  friend void serialize(Processor& proc, gset_impl<T>& x) {
    proc & x.set_;
  }

  intptr_t compare(const gset_impl<T>& other) const noexcept {
    if (set_ == other.set_)     return 0;
    else if (set_ < other.set_) return -1;
    else                        return 1;
  }

private:
  /// @private
  inline bool internal_emplace(const T& elem) {
    return std::get<1>(set_.emplace(elem));
  }
  std::set<T> set_; /// Set of elements
};

} // namespace cmrdt

//} // namespace <anonymous>

/// Implementation of a grow-only set (GSet)
template <class T>
struct gset : public cmrdt::gset_impl<T>,
              public caf::detail::comparable<gset<T>> {
  using value_type = T;
  /// Internal type of gset
  using internal_t = delta::gset_impl<T>;

  using interface = notifyable<gset<T>>;
  using base = typename notifyable<gset<T>>::base;
  using behavior_type = typename notifyable<gset<T>>::behavior_type;

  gset() = default;

  gset(std::set<T> set) : cmrdt::gset_impl<T>(std::move(set)) {
    // nop
  }
};

} // namespace types
} // namespace crdt
} // namespace caf

#endif // CAF_CRDT_TYPES_GSET_HPP
