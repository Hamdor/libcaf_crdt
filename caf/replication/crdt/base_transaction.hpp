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

#ifndef CAF_REPLICATION_CRDT_BASE_TRANSACTION_HPP
#define CAF_REPLICATION_CRDT_BASE_TRANSACTION_HPP

#include <chrono>
#include <string>

#include <iostream>

#include "caf/node_id.hpp"

namespace caf {
namespace replication {
namespace crdt {

/// Base type for transactions
struct base_transaction {
  /// Define used clock
  using clock = std::chrono::high_resolution_clock;
  /// Time point, provided by clock
  using time_point = std::chrono::time_point<clock>;

  base_transaction() = default;

  /// @param topic for this transaction
  /// @param creator the actor which created this transaction
  base_transaction(std::string topic, actor creator = {})
      : creator_{std::move(creator)}, time_{clock::now()},
        topic_{std::move(topic)} {
    // nop
  }

  /// @returns a actor handle of the creator
  inline const actor& creator() const { return creator_; }

  /// @returns node which generated this transaction
  inline node_id node() const { return creator_.node(); }

  /// @returns the timestamp for this operations
  inline const time_point& time() const { return time_; }

  inline void set_topic(const std::string& topic) {
    topic_ = topic;
  }

  /// @returns a string representing either the nanoseconds since epoch,
  /// or the ns since last boot.
  inline std::string ticks() const {
    std::stringstream ss;
    ss << time_.time_since_epoch().count();
    return ss.str();
  }

  /// @returns the topic of this transaction
  inline const std::string& topic() const { return topic_; }

  /// @returns `true`, override this function if your type supports
  /// a `empty()` implementation.
  virtual bool empty() const { return false; }

private:
  actor creator_;     /// Actor which created this transaction
  time_point time_;   /// Time point at transaction
  std::string topic_; /// Topic of transaction
};

} // namespace crdt
} // namesapce replication
} // namepsace caf

#endif // CAF_REPLICATION_CRDT_BASE_TRANSACTION_HPP
