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

#ifndef CAF_REPLICATION_DETAIL_REPLICATOR_HOOKS_HPP
#define CAF_REPLICATION_DETAIL_REPLICATOR_HOOKS_HPP

#include "caf/scoped_actor.hpp"

#include "caf/io/hook.hpp"

namespace caf {
namespace replication {
namespace detail {

///
class replicator_hooks : public io::hook {
public:
  replicator_hooks(actor_system& sys);

  /// Called whenever a handshake via a direct TCP connection succeeded.
  void new_connection_established_cb(const node_id& dest) override;

  /// Called whenever a message from or to a yet unknown node was received.
  void new_route_added_cb(const node_id&, const node_id& node) override;

  /// Called whenever a direct connection was lost.
  void connection_lost_cb(const node_id& dest) override;

  /// Called before middleman shuts down.
  void before_shutdown_cb() override;

private:

  void add_new_node(const node_id& nid);

  scoped_actor self_;
  actor_system& sys_;
};

} // namespace detail
} // namespace replication
} // namespace caf

#endif // CAF_REPLICATION_DETAIL_REPLICATOR_HOOKS_HPP
