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
using namespace caf::io;
using namespace caf::crdt;
using namespace caf::crdt::types;

namespace {

class config : public crdt_config {
public:
  config() {
    add_crdt<types::gset<int>>("gset<int>");
  }
};

void caf_main(actor_system& system, const config&) {
  auto repl = actor_cast<actor>(system.replicator().actor_handle());
  scoped_actor self{system};
  gset<int> set{self};
  set.insert(5);
  uri u{"gset<int>://test"};
  auto req = self->request(repl, infinite, write_all_atom::value, u,
                            make_message(set));
  req.receive(
    [&](write_succeed_atom) { aout(self) << "succeed...\n"; },
    [&](error&) {             aout(self) << "error...\n";   }
  );
  self->request(repl, infinite, read_all_atom::value, u).receive(
    [&](read_succeed_atom, const gset<int>& result) {
      aout(self) << "succeed...\n"
                 << "contains 5? " << (result.element_of(5) ? "yes" : "no")
                 << "\n";
    },
    [&](error&) { aout(self) << "error...\n"; }
  );
}

} // namespace <anonymous>

CAF_MAIN(io::middleman, crdt::replicator)
