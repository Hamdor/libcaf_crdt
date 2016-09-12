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

#include <string>

#include "caf/node_id.hpp"

namespace caf {
namespace replication {
namespace crdt {

struct base_transaction {
  /// @param topic for this transaction
  /// @param nid node which generated this transaction
  base_transaction(std::string topic, node_id nid) : topic_(std::move(topic)),
                                                     nid_(std::move(nid)) {
    // nop
  }

  /// @returns the topic of this transaction
  inline const std::string& topic() const { return topic_; }

  /// @returns node which generated this transaction
  inline const node_id& node() const { return nid_; }

  template <class Processor>
  friend void serialize(Processor& proc, base_transaction& x) {
    proc & x.topic_;
    proc & x.nid_;
  }

private:
  std::string topic_;
  node_id nid_;
};

} // namespace crdt
} // namesapce replication
} // namepsace caf

#endif // CAF_REPLICATION_CRDT_BASE_TRANSACTION_HPP
