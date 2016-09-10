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

#include <set>
#include <chrono>
#include <string>
#include <cstdlib>
#include <iostream>

#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "caf/replication/all.hpp"

using namespace std;
using namespace caf;
using namespace caf::replication;

/// Example worker, supports the `notifyabe_type<>` interface, to fetch data
/// from local top level replica.
template <class State>
class worker : public notifyable_type<State>::base {
public:
  worker(actor_config& cfg, std::string some_str)
      : notifyable_type<State>::base(cfg), id_string_(std::move(some_str)) {
    // nop
  }

  typename notifyable_type<State>::behavior_type make_behavior() override {
    return {
      [&](initial_atom, const State& state) {
        aout(this) << id_string_ << ": init" << endl;
        // Set initial state
        // The recieved state is configured to propagate update
        // to the local top level replica. This handler is just called once.
        state_ = std::move(state);
      },
      [&](notify_atom, const typename State::transactions_type& operations) {
        // Recieved operations from other replicas, apply the operations
        // to this local state and print values.
        state_.apply(operations);
        aout(this) << id_string_ << ": got " << operations.values().size()
                   << " " << "operations" << endl;
        for (const auto& value : state_.get_immutable())
          aout(this) << value << " ";
        aout(this) << endl;
      },
      after(std::chrono::seconds(1)) >> [&] {
        // Generate some random values, we assume, that other subscribers
        // will recieve these updates. This simulates some work.
        auto rnd = std::rand();
        aout(this) << id_string_ << ": generated rnd " << rnd << endl;
        state_.insert(rnd);
      }
    };
  }

private:
  std::string id_string_; /// String to identify this actor in cout
  State state_; /// State of worker
};

int main(int argc, char* argv[]) {
  // --- Create actor system
  actor_system system{actor_system_config{}.load<io::middleman>()
                                           .load<replication::replicator>()};
  // --- Spawn some workers
  auto worker1 = system.spawn<worker<crdt::gset<int>>>("worker1");
  auto worker2 = system.spawn<worker<crdt::gset<int>>>("worker2");
  // --- Subscribe to updates
  auto& repl = system.replicator();
  repl.subscribe<crdt::gset<int>>("/rand", worker1);
  repl.subscribe<crdt::gset<int>>("/rand", worker2);
}
