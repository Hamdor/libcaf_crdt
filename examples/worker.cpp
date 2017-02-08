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
#include "caf/crdt/all.hpp"

#include "caf/crdt/detail/distribution_layer.hpp"

#include "caf/crdt/lamport_clock.hpp"

using namespace std;
using namespace caf;
using namespace caf::crdt;

namespace std {

template <>
bool operator< (const std::pair<int, std::unordered_set<caf::crdt::uri>>& lhs,
                const std::pair<int, std::unordered_set<caf::crdt::uri>>& rhs) {
    return lhs.first < rhs.first;
};

} // namespace std

/// Example worker, supports the `notifyabe_type<>` interface, to fetch data
/// from local top level replica.
template <class State>
class worker : public State::base {
public:
  worker(actor_config& cfg, std::string some_str)
      : State::base(cfg), id_string_(std::move(some_str)) {
    // nop
  }

  typename State::behavior_type make_behavior() override {
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
                   << ", insert: ";
        for (auto& val : operations.values())
          aout(this) << val << " ";
        aout(this) << endl;
      },
      after(std::chrono::seconds(2)) >> [&] {
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

/// TODO: Implement a working config ....
/*struct cfg : public actor_system_config {
  using duration_t = decltype(std::chrono::milliseconds(0));
  cfg() {
    load<io::middleman>().
    load<replication::replicator>();
  }

  /// Creates a local root replica
  template <class T>
  cfg& add_replica(const std::string& topic, const duration_t& sync,
                   const duration_t& flush) {
    auto root_type_id = topic;
    root_type_id.append("/root");
    // Register actor types
    // https://actor-framework.readthedocs.io/en/latest/ConfiguringActorApplications.html?highlight=custom%20actor%20types#adding-custom-actor-types-experimental

    add_actor_type(root_type_id, caf::crdt::detail::root_replica_actor<T>);
    add_actor_type(topic, caf::crdt::detail::replica_actor<T>);
    //
    child_args.emplace_back(topic, "/", sync);
    //
    // TODO!!
    return *this;
  }

  /// Create hierachical replicas with given path
  template <class T, class Resync, class Flush>
  cfg& add_replica(const std::string& topic, const std::string& path,
                   const duration_t& flush) {
    // TODO: Path != "/" !!!
    return *this;
  }

  //                      topic,       path,       interval
  std::vector<std::tuple<std::string, std::string, duration_t>> child_args;
  std::vector<std::tuple<std::string, std::string, duration_t, duration_t> root_args;
};*/


int main(int argc, char* argv[]) {
  auto flush_interval  = std::chrono::seconds(1);
  auto resync_interval = std::chrono::seconds(10);
  std::string topic = "/rand";
  // --- Build local replica tree for topic "/rand"
  //            root
  //           /    \
  //       sub1      sub2
  //      /
  //  hello
  /*actor_system system{
    cfg{}.add_replica<types::gset<int>>(topic, "/", flush_interval,
                                       resync_interval)
         .add_replica<types::gset<int>>(topic, "/sub1", flush_interval,
                                       resync_interval)
         .add_replica<types::gset<int>>(topic, "/sub1/hello", flush_interval,
                                       resync_interval)
         .add_replica<types::gset<int>>(topic, "/sub2", flush_interval,
                                       resync_interval)
         .load<io::middleman>()
         .load<replication::replicator>()};*/
  crdt_config cfg{};
  cfg.load<io::middleman>().load<crdt::replicator>();
  cfg.add_crdt<types::gset<int>>("gset<int>");
  actor_system system{cfg};
  // --- Spawn some workers
  // Spawn a new tree:
  //       root
  //      /   \
  //   sub1    sub2
  //   /
  // subsub1
  auto worker1 = system.spawn<worker<types::gset<int>>>("worker1");
  auto worker2 = system.spawn<worker<types::gset<int>>>("worker2");
  auto worker3 = system.spawn<worker<types::gset<int>>>("worker3");

  uri u{"gset<int>://rand"};

  auto& repl = system.replicator();
  repl.subscribe<types::gset<int>>(u, /*"/sub2",*/ worker1);
  repl.subscribe<types::gset<int>>(u, /*"/sub1/subsub1",*/ worker2);
  repl.subscribe<types::gset<int>>(u, /*"/sub1",*/ worker3);

  types::gmap<node_id, std::pair<int, std::unordered_set<uri>>> some_map;
  some_map.assign(node_id{}, make_pair(12, std::unordered_set<uri>{}));
  some_map.assign(node_id{}, make_pair(11, std::unordered_set<uri>{}));

/*
  // gset<int>://videos/<ids>
  // gset<string>://videos/<ids>
  // /likes/videos/<id>
  // /likes/comments/<vid>/<id>
  auto topic = system.replicator().topic<std::set<uri>>("all?t=gset<int>://videos/*");
  auto topic = system.replicator().topic<types::gset<int>>("gset<int>://rand");
  self.send(worker1, topic);
  spawn([=](event_based_actor* self) {
    self->join(topic, ""); // root (default)
    self->join(topic, "w1"); // root.w1
    self->join(topic, "w1.g1"); // root.w1.g1
    self->send(topic, transaction{insert, 42});
  });
*/
}
