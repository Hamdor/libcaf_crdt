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

#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "caf/crdt/all.hpp"

using namespace caf;
using namespace caf::crdt;

namespace {

constexpr int nr_spawn = 5;
constexpr int inc_by   = 10;
constexpr int expected = nr_spawn * inc_by *2;

struct port_dummy : public event_based_actor {
  using event_based_actor::event_based_actor;
};

class incrementer : public types::gcounter<int>::base {
public:
  incrementer(actor_config& cfg)
    : types::gcounter<int>::base(cfg),
      state_(this, "gcounter<int>://counter") {
    // nop
  }

protected:
  typename types::gcounter<int>::behavior_type make_behavior() override {
    state_.increment_by(inc_by);
    return {
      [&](notify_atom, const types::gcounter<int>& t) {
        state_.merge(t);
        if (state_.count() == expected) {
          aout(this) << "Count is: " << state_.count() << " ==> quit()\n";
          quit();
        }
      }
    };
  }

private:
  types::gcounter<int> state_;
};

class config : public crdt_config {
public:
  config() {
    add_crdt<types::gcounter<int>>("gcounter<int>");
  }
};

void caf_main(actor_system& system, const config& cfg) {
  config conf{};
  conf.load<io::middleman>().load<crdt::replicator>();
  actor_system system2{conf};
  // -- Publish two dummies
  auto port1 = *system.middleman().publish(system.spawn<port_dummy>(), 0);
  auto port2 = *system2.middleman().publish(system.spawn<port_dummy>(), 0);
  std::this_thread::sleep_for(std::chrono::seconds(1));
  // ----------------------
  // -- Connect to nodes
  {
    scoped_actor self{system};
    self->send(system.middleman().actor_handle(), connect_atom::value,
               "127.0.0.1", port2);
    scoped_actor self2{system2};
    self2->send(system.middleman().actor_handle(), connect_atom::value,
                "127.0.0.1", port1);
  }
  std::this_thread::sleep_for(std::chrono::seconds(1));
  // ----------------------
  for (int i = 0; i < nr_spawn; ++i) {
   system.spawn<incrementer>();
   system2.spawn<incrementer>();
  }
}

} // namespace <anonymous>

CAF_MAIN(io::middleman, crdt::replicator)
