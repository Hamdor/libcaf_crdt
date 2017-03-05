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

#ifndef CAF_CRDT_TYPES_BASE_DATATYPE_HPP
#define CAF_CRDT_TYPES_BASE_DATATYPE_HPP

#include <string>

#include "caf/message.hpp"

#include "caf/crdt/atom_types.hpp"

#include "caf/actor_ostream.hpp"

namespace caf {
namespace crdt {
namespace types {

///
struct base_datatype {
  /// @private
  base_datatype() = default;

  ///
  template <class ActorType>
  base_datatype(const ActorType& owner, const std::string& topic)
    : owner_(actor_cast<actor>(owner)), topic_(topic) {
    auto hdl = owner_.home_system().replicator().actor_handle();
    send_as(owner_, hdl, subscribe_atom::value, uri{topic});
  }

  virtual ~base_datatype() {
    // If owner_ is no longer valid, the actor system got shut down and it is
    // no longer possible nor necessary to unsubscribe
    if (owner_) {
      auto hdl = owner_.home_system().replicator().actor_handle();
      send_as(owner_, hdl, unsubscribe_atom::value, uri{topic_});
    }
  }

  /// @returns topic of this state
  inline std::string topic() const { return topic_.str(); }

  /// @returns the owner of this state
  inline const actor& owner() const { return owner_; }

  /// @private
  template <class Processor>
  friend void serialize(Processor& proc, base_datatype& x) {
    proc & x.topic_;
  }

protected:
  ///
  template <class Data>
  void publish(const Data& data) const {
    auto hdl = owner_.home_system().replicator().actor_handle();
    send_as(owner_, hdl, topic_, make_message(data));
  }

private:
  actor owner_; /// Owner of this state
  uri topic_;   /// Topic for this datatype
};

} // namespace types
} // namespace crdt
} // namespace caf

#endif // CAF_CRDT_TYPES_BASE_DATATYPE_HPP
