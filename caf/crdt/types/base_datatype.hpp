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

#ifndef CAF_CRDT_TYPES_BASE_DATATYPE_HPP
#define CAF_CRDT_TYPES_BASE_DATATYPE_HPP

#include "caf/send.hpp"
#include "caf/message.hpp"
#include "caf/detail/type_traits.hpp"

#include "caf/crdt/atom_types.hpp"

#include <string>

namespace caf {
namespace crdt {
namespace types {

#define DECL_CRDT_CTORS(name)                                                  \
  template <                                                                   \
    class U,                                                                   \
    class = caf::detail::enable_if_t<                                          \
      std::is_same<name, typename std::decay<U>::type>::value                  \
    >                                                                          \
  >                                                                            \
  name(U&& other) {                                                            \
    *this = std::forward<U>(other);                                            \
  }                                                                            \
                                                                               \
  template <                                                                   \
    class U,                                                                   \
    class = caf::detail::enable_if_t<                                          \
      !std::is_same<name, typename std::decay<U>::type>::value                 \
    >                                                                          \
  >                                                                            \
  name(U&& owner, std::string id = {})                                         \
    : base_datatype(std::forward<U>(owner), std::move(id)) {                   \
  }                                                                            \
                                                                               \
  name() = default;                                                            \

/// Base class for CRDTs
class base_datatype {
public:
  /// @private
  base_datatype() = default;

  /// @param owner A actor handle to state the owning actor
  /// @param id Replica-ID for this instance
  template <class ActorType>
  base_datatype(const ActorType& owner, const std::string& id)
    : owner_(actor_cast<actor>(owner)), id_(id) {
    if (!id.empty()) {
      auto hdl = owner_.home_system().replicator().actor_handle();
      send_as(owner_, hdl, subscribe_atom::value, uri{id});
    }
  }

  virtual ~base_datatype() {
    if (id_.valid() && owner_) {
      auto hdl = owner_.home_system().replicator().actor_handle();
      send_as(owner_, hdl, unsubscribe_atom::value, uri{id_});
    }
  }

  /// @returns id of this state
  inline std::string id() const { return id_.to_string(); }

  /// @returns the owner of this state
  inline const actor& owner() const { return owner_; }

protected:
  /// Publishes a delta crdt state to the replicator
  /// @param data the delta to be pushed to replicator
  template <class Data>
  void publish(const Data& data) const {
    if (!id_.valid())
      return;
    auto hdl = owner_.home_system().replicator().actor_handle();
    send_as(owner_, hdl, id_, make_message(data));
  }

private:
  actor owner_; /// Owner of this state
  uri id_;      /// Replic-ID
};

} // namespace types
} // namespace crdt
} // namespace caf

#endif // CAF_CRDT_TYPES_BASE_DATATYPE_HPP
