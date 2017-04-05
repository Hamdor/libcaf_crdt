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

#include "caf/crdt/detail/replicator_hooks.hpp"

#include "caf/crdt/replicator.hpp"

using namespace caf::crdt::detail;

replicator_hooks::replicator_hooks(actor_system& sys)
    : io::hook(sys),
      self_(sys, true),
      sys_(sys) {
  // nop
}

void replicator_hooks::new_connection_established_cb(const node_id& node) {
  new_connection(node);
}

void replicator_hooks::new_route_added_cb(const node_id&, const node_id& node) {
  new_connection(node);
}

void replicator_hooks::connection_lost_cb(const node_id& node) {
  auto hdl = sys_.replicator().actor_handle();
  self_->send(hdl, connection_lost_atom::value, node);
}

void replicator_hooks::new_connection(const node_id& node) {
  auto hdl = sys_.replicator().actor_handle();
  self_->send(hdl, new_connection_atom::value, node);
}
