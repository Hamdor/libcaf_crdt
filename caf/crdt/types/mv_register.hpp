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

#ifndef CAF_CRDT_TYPES_MV_REGISTER_HPP
#define CAF_CRDT_TYPES_MV_REGISTER_HPP

#include "caf/crdt/vector_clock.hpp"

#include "caf/crdt/types/base_datatype.hpp"

namespace caf {
namespace crdt {
namespace types {

/// Multi-Value-Register (MV-Register)
template <class T>
class mv_register : public base_datatype {

  mv_register(std::set<std::tuple<T, vector_clock>> value,
              vector_clock&& clk)
    : register_{value}, clk_{std::move(clk)} {
    // nop
  }

  mv_register(std::set<std::tuple<T, vector_clock>>&& set, vector_clock clk)
    : register_{std::move(set)}, clk_{std::move(clk)} {
    // nop
  }

public:
  using value_type = T;

  DECL_CRDT_CTORS(mv_register)

  /// Set a new element to the register
  /// @param value to set
  void set(const T& value) {
    auto clock = clk_.increment(owner());
    register_ = {std::make_tuple(value, clock)};
    publish(mv_register{register_, std::move(clock)});
  }

  /// @returns the current set of elements
  const std::set<std::tuple<T, vector_clock>>& get_set() const {
    return register_;
  }

  /// @returns the current element
  ///          if there concurrent elements, none will be returned
  ///          if there is no current element, the element will be returned
  optional<T> get() const {
    if (register_.size() > 1)
      return none;
    return std::get<0>(*register_.begin());
  }

  ///
  mv_register merge(const mv_register& other) {
    std::set<std::tuple<T, vector_clock>> delta;
    switch(other.clk_.compare(clk_)) {
      case greater:
        register_ = other.register_;
        delta     = other.register_;
        break;
      case smaller:
      case equal:
        return {};
      case concurrent:
        register_.insert(other.register_.begin(), other.register_.end());
        std::set_symmetric_difference(register_.begin(), register_.end(),
                                      other.register_.begin(),
                                      other.register_.end(),
                                      std::inserter(delta, delta.begin()));
        break;
    }
    clk_.merge(other.clk_);
    return {std::move(delta), clk_};
  }

  /// @private
  template <class Processor>
  friend void serialize(Processor& proc, mv_register<T>& x) {
    proc & x.register_;
    proc & x.clk_;
  }

private:
  std::set<std::tuple<T, vector_clock>> register_;
  vector_clock clk_;
};

} // namespace types
} // namespace crdt
} // namespace caf

#endif // CAF_CRDT_TYPES_MV_REGISTER_HPP
