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

#include "caf/string_algorithms.hpp"
#include "caf/actor_system_config.hpp"

#include "caf/detail/type_list.hpp"

#include "caf/io/middleman.hpp"

#include "caf/replication/replicator.hpp"

#include "caf/replication/detail/register_types.hpp"
#include "caf/replication/detail/replicator_hooks.hpp"

#include <exception>

using namespace std;
using namespace caf;
using namespace caf::replication;

void replicator::start() {
  manager_ = make_replicator_actor(system_);
  system_.registry().put(replicator_atom::value,
                         actor_cast<strong_actor_ptr>(manager_));
}

void replicator::stop() {
  std::cout << __PRETTY_FUNCTION__ << std::endl;
  system_.registry().erase(replicator_atom::value);
  scoped_actor self{system(), true};
  self->monitor(manager_);
  self->send_exit(manager_, exit_reason::user_shutdown);
  self->wait_for(manager_);
  destroy(manager_);
}

void replicator::init(actor_system_config& cfg) {
  // TODO: Add all message types... and remove outdated
  cfg.add_hook_type<detail::replicator_hooks>().
      add_message_type<std::unordered_map<std::string, message>>("merged_updates").
      add_message_type<std::unordered_map<std::string, std::vector<message>>>("unmerged_messages");
  // Default supportet replica types
  detail::register_types{cfg}();
}

actor_system::module::id_t replicator::id() const {
  return actor_system::module::replicator;
}

void* replicator::subtype_ptr() {
  return this;
}

actor_system::module* replicator::make(actor_system& sys,
                                       caf::detail::type_list<>) {
  return new replicator{sys};
}

replicator_actor replicator::actor_handle() {
  return manager_;
}

replicator::replicator(actor_system& sys) : system_(sys),
                                            manager_{} {
  // nop
}

replicator::~replicator() {
  // nop
}
