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

#include "caf/replication/detail/replicator_hooks.hpp"

#include "caf/replication/replicator.hpp"

using namespace caf::replication::detail;

replicator_hooks::replicator_hooks(actor_system& sys)
    : io::hook(sys),
      self_(sys, true),
      sys_(sys) {
  // nop
}

void replicator_hooks::new_connection_established_cb(const node_id& node) {
  auto hdl = sys_.replicator().actor_handle();
  if (!hdl.unsafe())
    self_->send(hdl, new_direct_con::value, node);
}

void replicator_hooks::new_route_added_cb(const node_id&, const node_id& node) {
  auto hdl = sys_.replicator().actor_handle();
  if (!hdl.unsafe())
    self_->send(hdl, new_indirect_con::value, node);
}

void replicator_hooks::connection_lost_cb(const node_id& dest) {
  auto hdl = sys_.replicator().actor_handle();
  if (!hdl.unsafe())
    self_->send(hdl, con_lost::value, dest);
}

void replicator_hooks::before_shutdown_cb() {
  // TODO?
}
