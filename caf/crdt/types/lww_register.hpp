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

#ifndef CAF_CRDT_TYPES_LWW_REGISTER_HPP
#define CAF_CRDT_TYPES_LWW_REGISTER_HPP

#include "caf/crdt/vector_clock.hpp"

#include "caf/crdt/types/base_datatype.hpp"

namespace caf {
namespace crdt {
namespace types {

/// Last Writer Wins Register (LWW-Register) implementation as delta-CRDT
template <class T>
class lww_register : public base_datatype {
  /// @private
  lww_register(vector_clock clk, actor setter, T value)
    : clk_{std::move(clk)}, setter_{std::move(setter)},
      value_{std::move(value)} {
    // nop
  }

public:
  using value_type = T;

  DECL_CRDT_CTORS(lww_register)

  /// Merge another lww_register state into this
  /// @param other delta-CRDT to merge into this
  /// @returns a delta lww_register<T>
  lww_register<T> merge(const lww_register<T>& other) {
    auto transfer = [&](const lww_register<T>& value) {
      this->clk_    = value.clk_;
      this->setter_ = value.setter_;
      this->value_  = value.value_;
      return value;
    };
    switch(clk_.compare(other.clk_)) {
      case greater:
        return transfer(other);
      case equal:
      case smaller:
        return {};
      case concurrent:
        if (setter_ == other.setter_) return {};
        else if (setter_ > other.setter_) return {};
        else if (setter_ < other.setter_) return transfer(other);
    }
    return {};
  }

  /// Set a new element to the register
  /// @param value to set
  void set(const T& value) {
    clk_ = clk_.increment(owner());
    setter_ = owner();
    value_ = value;
    publish(lww_register<T>{clk_, setter_, value_});
  }

  /// Move a new element to the register
  /// @param value to set
  void set(T&& value) {
    clk_ = clk_.increment(owner());
    setter_ = owner();
    value_ = std::move(value);
    publish(lww_register<T>{clk_, setter_, value_});
  }

  /// @returns the current element
  inline const T& get() const { return value_; }

  /// @private
  template <class Processor>
  friend void serialize(Processor& proc, lww_register<T>& x) {
    proc & x.clk_;
    proc & x.setter_;
    proc & x.value_;
  }

  /// @private
  inline size_t empty() const { return clk_.count() == 0; }

private:
  vector_clock clk_;  /// Timestamp of current element
  actor setter_;      /// Setter of current element
  T value_;           /// Current element
};

} // namespace types
} // namespace crdt
} // namespace caf

#endif // CAF_CRDT_TYPES_LWW_REGISTER_HPP
