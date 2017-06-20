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

constexpr int to_spawn = 4;

class config : public crdt_config {
public:
  config() {
    add_crdt<types::gset<int>>("gset<int>");
  }
};

struct state {
  state(event_based_actor* self) : crdt{self, "gset<int>://set"} {
    // nop
  }
  gset<int> crdt;
};

void actor_fun(stateful_actor<state>* self) {
  self->become(
    [=](int value) {
      self->state.crdt.insert(value);
    },
    [=](notify_atom, const gset<int>& other) {
      self->state.crdt.merge(other);
      if (self->state.crdt.size() == to_spawn) {
        aout(self) << "got all entries... exiting...\n";
        self->quit();
      }
    }
  );
}

void caf_main(actor_system& system, const config&) {
  for (int i = 0; i < to_spawn; ++i)
    anon_send(system.spawn(actor_fun), i);
}

} // namespace <anonymous>

CAF_MAIN(io::middleman, crdt::replicator)
