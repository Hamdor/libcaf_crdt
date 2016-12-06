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

#include "caf/message.hpp"

#include "caf/replication/atom_types.hpp"

namespace caf {
namespace replication {
namespace crdt {

///
//template <class T>
class base_datatype {
public:
  /// Default constructor
  base_datatype() : owner_(unsafe_actor_handle_init)
                  , parent_(unsafe_actor_handle_init) {
    // nop
  }

  /*T merge(message msg) {
    T result;
    msg.apply([&](const T& unpacked) {
      result = std::move(this->merge(unpacked));
    },
    others >> [] {
      // TODO: ERROR!
    });
    return result;
  }*/

  /// @returns topic of this state
  inline const std::string& topic() const { return topic_; }

  /// @returns owning actor
  inline const actor& owner() const { return owner_; }

  /// @returns local node
  inline node_id node() const { return owner_ ? owner_.node() : node_id{}; }

  /// @private
  inline void set_owner(actor act) { owner_ = std::move(act); }

  /// @private
  inline void set_parent(actor act) { parent_ = std::move(act); }

  /// @private
  inline void set_topic(std::string topic) { topic_ = std::move(topic); }

protected:
  /// Propagate transaction to our parent
  template <class Data>
  void publish(const Data& data) const {
    if (!parent_.unsafe())
      send_as(owner_, parent_, publish_atom::value, data);
  }

private:
  actor owner_; /// Owning actor of this state
  actor parent_; /// Parent of owning actor
  std::string topic_; /// Topic for this datatype
};


} // namespace crdt
} // namespace replication
} // namespace caf

#endif // CAF_REPLICATION_CRDT_BASE_DATATYPE_HPP
