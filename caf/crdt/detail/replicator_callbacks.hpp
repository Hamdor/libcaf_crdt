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

#ifndef CAF_CRDT_DETAIL_REPLICATOR_CALLBACKS_HPP
#define CAF_CRDT_DETAIL_REPLICATOR_CALLBACKS_HPP

#include "caf/scoped_actor.hpp"

#include "caf/io/hook.hpp"

namespace caf {
namespace crdt {
namespace detail {

/// IO-Hooks used by replicator
class replicator_callbacks : public io::hook {
public:
  replicator_callbacks(actor_system& sys);

  /// Called whenever a handshake via a direct TCP connection succeeded.
  void new_connection_established_cb(const node_id& dest) override;

  /// Called whenever a message from or to a yet unknown node was received.
  void new_route_added_cb(const node_id&, const node_id& node) override;

  /// Called whenever a direct connection was lost.
  void connection_lost_cb(const node_id& dest) override;

private:
  void on_new_connection(const node_id& node);

  scoped_actor self_;
  actor_system& sys_;
};

} // namespace detail
} // namespace crdt
} // namespace caf

#endif // CAF_CRDT_DETAIL_REPLICATOR_CALLBACKS_HPP
