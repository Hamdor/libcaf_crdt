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

#define CAF_SUITE spawn_replica
#include "caf/test/unit_test.hpp"

#include "caf/all.hpp"
#include "caf/crdt/all.hpp"

using namespace caf;
using namespace caf::crdt;
using namespace caf::crdt::types;
using namespace std::chrono;

namespace {

class config : public crdt_config {
public:
  config() {
    add_crdt<gset<float>>("gset<float>");
  }

};

struct fixture {
  fixture() : system{cfg} {
    // nop
  }

  config cfg;
  actor_system system;
};

} // namespace <anonymous>


CAF_TEST_FIXTURE_SCOPE(spawn_replica_test, fixture)

CAF_TEST(spawn_fail) {
  auto repl = system.replicator().actor_handle();
  scoped_actor self{system};
  bool failed = false;
  self->request(repl, seconds(1), subscribe_atom::value,
                uri{"gset<int>://bla"}).receive(
    [] {
      // nop
    },
    [&](error&) { failed = true; }
  );
  CAF_CHECK(failed);
}

CAF_TEST(spawn_ok) {
  auto repl = system.replicator().actor_handle();
  scoped_actor self{system};
  bool failed = false;
  self->request(repl, seconds(1), subscribe_atom::value,
                uri{"gset<float>://bla"}).receive(
    [] {
      // nop
    },
    [&](error&) { failed = true; }
  );
  CAF_CHECK(!failed);
}

CAF_TEST_FIXTURE_SCOPE_END()
