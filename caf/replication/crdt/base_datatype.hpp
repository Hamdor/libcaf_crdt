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

#ifndef CAF_REPLICATION_CRDT_BASE_DATATYPE_HPP
#define CAF_REPLICATION_CRDT_BASE_DATATYPE_HPP

#include <string>

#include "caf/replication/atom_types.hpp"
#include "caf/replication/publish_subscribe.hpp"

namespace caf {
namespace replication {
namespace crdt {

///
class base_datatype {
public:
  /// Default constructor
  base_datatype() : owner_(unsafe_actor_handle_init)
                  , parent_(unsafe_actor_handle_init) {
    // nop
  }

  ///
  template <class Data>
  void publish(const Data& data) const {
    if (!parent_.unsafe())
      send_as(owner_, parent_, publish_atom::value, data);
  }

  /// @returns topic of this state
  inline const std::string& topic() { return topic_; }

  /// @private
  inline void set_owner(actor act) { owner_ = std::move(act); }

  /// @private
  inline void set_parent(actor act) { parent_ = std::move(act); }

  /// @private
  inline void set_topic(std::string topic) { topic_ = std::move(topic); }

private:
  actor owner_;
  actor parent_;
  std::string topic_;
};


} // namespace crdt
} // namespace replication
} // namespace caf

#endif // CAF_REPLICATION_CRDT_BASE_DATATYPE_HPP
