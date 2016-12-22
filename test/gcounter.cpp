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

#define CAF_SUITE gcounter
#include "caf/test/unit_test.hpp"

#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "caf/replication/all.hpp"

using namespace std;
using namespace caf;
using namespace caf::replication;

namespace {

template <class State>
class worker : public notifyable<State>::base {
public:
  worker(actor_config& cfg, std::string some_str)
      : notifyable<State>::base(cfg), id_string_(std::move(some_str)) {
    // nop
  }

  typename notifyable<State>::behavior_type make_behavior() override {
    return {
      [&](initial_atom, State& state) {
        aout(this) << id_string_ << ": init" << endl;
        // Set initial state
        // The recieved state is configured to propagate update
        // to the local top level replica. This handler is just called once.
        state_ = std::move(state);
      },
      [&](notify_atom, const typename State::transaction_t& operations) {
        // Recieved operations from other replicas, apply the operations
        // to this local state and print values.
        state_.apply(operations);
        aout(this) << id_string_ << ": " << operations.ticks() << "ns"
                   << ", insert: " << ", topic: " << operations.topic() << std::endl;
        for (auto& val : operations.values())
          aout(this) << val << " ";
        aout(this) << endl;
      },
      after(std::chrono::seconds(1)) >> [&] {
        if (tick_count_++ == 10) this->quit();
        // Generate some random values, we assume, that other subscribers
        // will recieve these updates. This simulates some work.
        int rnd = std::rand() + *(id_string_.end()-1);
        aout(this) << id_string_ << ": generated rnd " << rnd << endl;
        state_.insert(rnd);
      }
    };
  }

private:
  std::string id_string_; /// String to identify this actor in cout
  State state_; /// State of worker
  int tick_count_;
};

} // namespace <anonymous>

CAF_TEST(test) {
  uri u{"gset<int>://rand"};
  auto cfg = replicator_config{};
  cfg.load<io::middleman>().load<replication::replicator>();
  cfg.add_replica_type<crdt::gset<int>>("gset<int>");
  actor_system system{cfg};

  auto worker1 = system.spawn<worker<crdt::gset<int>>>("worker1");
  auto worker2 = system.spawn<worker<crdt::gset<int>>>("worker2");
  auto worker3 = system.spawn<worker<crdt::gset<int>>>("worker3");
  // ...
  auto& repl = system.replicator();
  repl.subscribe<crdt::gset<int>>(u, worker1);
  repl.subscribe<crdt::gset<int>>(u, worker2);
  repl.subscribe<crdt::gset<int>>(u, worker3);
  // TODO: Add collector and compare if all states are equiv...
}
