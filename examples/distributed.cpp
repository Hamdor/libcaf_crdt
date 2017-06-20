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

#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "caf/crdt/all.hpp"

using namespace caf;
using namespace caf::crdt;
using namespace caf::crdt::types;

namespace {

constexpr int nr_spawn = 10;
constexpr int inc_by   = 10;
constexpr int number_of_nodes = 2;
constexpr int expected = nr_spawn * inc_by * number_of_nodes;

class port_dummy : public event_based_actor {
public:
  using event_based_actor::event_based_actor;
};

class incrementer : public notifiable<gcounter<int>>::base {
public:
  incrementer(actor_config& cfg)
    : notifiable<gcounter<int>>::base(cfg),
      state_(this, "gcounter<int>://counter") {
    // nop
  }

protected:
  notifiable<gcounter<int>>::behavior_type make_behavior() override {
    state_.increment_by(inc_by);
    return {
      [&](notify_atom, const gcounter<int>& t) {
        state_.merge(t);
        if (state_.count() == expected) {
          aout(this) << "Count is: " << state_.count() << " ==> quit()\n";
          quit();
        }
      }
    };
  }

private:
  gcounter<int> state_;
};

class config : public crdt_config {
public:
  config() : crdt_config() {
    add_crdt<gcounter<int>>("gcounter<int>");
  }
};

void caf_main(actor_system& system, const config&) {
  config conf{};
  actor_system system2{conf};
  auto port1 = system.middleman().open(0);
  auto port2 = system2.middleman().open(0);
  if (!port1 || !port2)
    return;
  system.middleman().connect("localhost", *port2);
  system2.middleman().connect("localhost", *port1);
  for (int i = 0; i < nr_spawn; ++i) {
    system.spawn<incrementer>();
    system2.spawn<incrementer>();
  }
}

} // namespace <anonymous>

CAF_MAIN(io::middleman, crdt::replicator)
