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

#ifndef CAF_REPLICATION_CRDT_LWW_REGISTER_HPP
#define CAF_REPLICATION_CRDT_LWW_REGISTER_HPP

#include "caf/node_id.hpp"

#include "caf/replication/lamport_clock.hpp"

#include "caf/replication/crdt/base_datatype.hpp"
#include "caf/replication/crdt/base_transaction.hpp"

// TODO: Add unit test for this type!
// TODO: Reduce this implementation,
//       for this type it is possible to use just one class

namespace caf {
namespace replication {
namespace crdt {

//namespace {

/// A LWW Reg support the following mutable operations
enum class lww_reg_operations {
  none,
  set
};

/// Describes a transation for lww_reg<> as CmRDT
template <class T>
struct lww_reg_transaction : public base_transaction {
  using operation_t = lww_reg_operations;

  lww_reg_transaction(std::string topic, operation_t operation,
                      lamport_clock clk, node_id setter, T value)
    : base_transaction(std::move(topic)),
      op_(std::move(operation)),
      clk_(std::move(clk)),
      setter_(std::move(setter)),
      value_(std::move(value)) {
    // nop
  }

  /// Construct a new transaction
  lww_reg_transaction(std::string topic, actor owner, operation_t operation,
                      lamport_clock clk, node_id setter, T value)
    : base_transaction(std::move(topic), std::move(owner)),
      op_(std::move(operation)),
      clk_(std::move(clk)),
      setter_(std::move(setter)),
      value_(std::move(value)) {
    // nop
  }

  /// Returns the operation of this transaction
  inline const operation_t& operation() const { return op_; }

  /// Returns the value of this transaction
  inline const T& value() const { return value_; }

  /// Returns the timestamp of this transaction
  inline uint64_t timestamp() const { return clk_.time(); }

  /// Returns the setter of the value (origin node of value)
  inline const node_id& setter() const { return setter_; }

private:
  operation_t op_;
  lamport_clock clk_;
  node_id setter_;
  T value_;
};

namespace delta {

/// This state is hold by top level replica, the CRDT is implemented as
/// delta-CRDT.
/// @private
template <class T>
struct lww_reg_impl {
  using operator_t = lww_reg_operations;
  using transaction_t = lww_reg_transaction<T>;

  /// Default constructor
  lww_reg_impl() = default;

  lww_reg_impl(uint64_t time, node_id nid, T value)
      : clk_{time}, setter_{nid}, value_{std::move(value)} {
    // nop
  }

  /// Apply transaction from a local subscriber to top level replica
  /// @param history a transaction
  /// @return a delta-CRDT which represent the delta
  lww_reg_impl<T> apply(const transaction_t& history) {
    if (history.operation() != operator_t::set)
      return {};
    if (history.timestamp() > clk_
         || (history.timestamp() == clk_ && history.setter() > setter_)) {
      clk_ = {history.timestamp()};
      setter_ = history.setter();
      value_ = history.value();
      return {clk_.time(), setter_, value_};
    }
    return {};
  }

  /// Merge function, for this type it is simple
  /// @param other delta-CRDT to merge into this
  /// @returns a delta gset<T>
  lww_reg_impl<T> merge(const lww_reg_impl<T>& other) {
    if (other.clk_ > clk_ || (other.clk_ == clk_ && other.setter_ > setter_)) {
      clk_ = other.clk_;
      setter_ = other.setter_;
      value_ = other.value_;
      return {clk_.time(), setter_, value_};
    }
    return {};
  }

  /// This is used to convert this delta-CRDT to CmRDT transactions
  /// @param topic for this transaction
  /// @param creator these transactions where done
  transaction_t get_cmrdt_transactions(const std::string& topic) const {
    return {topic, operator_t::set, clk_, setter_, value_};
  }

  /// @returns `true` if the state is empty
  ///          `false` otherwise
  inline bool empty() const { return clk_.time() == 0; }

  /// @private
  template <class Processor>
  friend void serialize(Processor& proc, lww_reg_impl<T>& x) {
    proc & x.clk_;
    proc & x.setter_;
    proc & x.value_;
  }

private:
  lamport_clock clk_;
  node_id setter_;
  T value_;
};

} // namespace delta

namespace cmrdt {

/// Last writer wins register (LWW-Reg) as CmRDT.
template <class T>
class lww_reg_impl : public base_datatype {
public:
  using operator_t = lww_reg_operations;
  /// Mutable operations will trigger this type
  using transaction_t = lww_reg_transaction<T>;

  lww_reg_impl() = default;

  /// Set a element
  /// @param elem element to set
  /// @returns a transaction
  transaction_t set(const T& elem) {
    value_ = elem;
    auto trans = transaction_t{topic(), owner(), operator_t::set,
                               {clk_.increment()}, node(), elem};
    publish(trans);
    return trans;
  }

  /// @returns the current value of the register
  const T& get() const { return value_; }

  /// Checks if the register contains an equal value
  bool equal(const T& other) const {
    return value_ == other;
  }

  /// Apply transaction to local CmRDT type
  /// @param history to apply
  void apply(const transaction_t& history) {
    if (history.operation() == operator_t::none)
      return;
    if (history.timestamp() > clk_
      || (history.timestamp() == clk_ && history.setter() > setter_)) {
      clk_ = {history.timestamp()};
      setter_ = history.setter();
      value_  = history.value();
    }
  }

  /// @private
  template <class Processor>
  friend void serialize(Processor& proc, lww_reg_impl<T>& x) {
    proc & x.clk_;
    proc & x.setter_;
    proc & x.value_;
  }

private:
  lamport_clock clk_;
  node_id setter_;
  T value_;
};

} // namespace cmrdt

//} // namespace <anonymous>

/// Implementation of a Last writer wins register (LWW-Reg)
template <class T>
struct lww_reg : public cmrdt::lww_reg_impl<T>,
                 public caf::detail::comparable<lww_reg<T>> {

  lww_reg() = default;

  /// Internal type of gset
  using internal_t = delta::lww_reg_impl<T>;
};

} // namespace crdt
} // namespace replication
} // namespace caf

#endif // CAF_REPLICATION_CRDT_LWW_REGISTER_HPP
