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

#define CAF_SUITE read_write_rules
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
    add_crdt<gset<int>>("gset<int>");
  }

};

struct fixture {
  fixture() : system{cfg} {
    // nop
  }

  config cfg;
  actor_system system;
};

template <class Atom, class Type>
void test_read(actor_system& system, const std::string& id, bool expect_fail,
               size_t k = 0) {
  scoped_actor self{system};
  auto repl = actor_cast<actor>(system.replicator().actor_handle());
  bool err = true;
  auto req = k == 0 ? self->request(repl, seconds(1), Atom::value, uri{id})
                    : self->request(repl, seconds(1), Atom::value, k, uri{id});
  req.receive(
    [&](read_succeed_atom, const Type&) { err = false; },
    [&](error)                          { err = true; }
  );
  CAF_CHECK(expect_fail == err);
}

template <class Atom, class Type>
void test_write(actor_system& system, const std::string& id, bool expect_fail,
                const Type& type, size_t k = 0) {
  scoped_actor self{system};
  auto repl = actor_cast<actor>(system.replicator().actor_handle());
  bool err = true;
  auto req = k == 0 ? self->request(repl, seconds(1), Atom::value, uri{id}, type)
                    : self->request(repl, seconds(1), Atom::value, k, uri{id}, type);
  req.receive(
    [&](write_succeed_atom) { err = false; },
    [&](error)              { err = true; }
  );
  CAF_CHECK(expect_fail == err);
}

} // namespace <anonymous>


CAF_TEST_FIXTURE_SCOPE(read_write_rules_test, fixture)

CAF_TEST(write_local) {
  test_write<write_local_atom, gset<int>>(system, "gset<int>", false, gset<int>{});
  test_write<write_local_atom, gset<float>>(system, "gset<float>", true, gset<float>{});
  test_write<write_local_atom, gset<int>>(system, "gset<float>", true, gset<int>{});
}

CAF_TEST(write_k) {

}

CAF_TEST(write_majority) {

}

CAF_TEST(write_all) {

}

CAF_TEST(read_local) {
  test_read<read_local_atom, gset<int>>(system, "gset<int>", false);
  test_read<read_local_atom, gset<float>>(system, "gset<float>", true);
  test_read<read_local_atom, gset<int>>(system, "gset<float>", true);
}

CAF_TEST(read_k) {
  size_t k = 12;
  test_read<read_k_atom, gset<int>>(system, "gset<int>", false, k);
  test_read<read_k_atom, gset<float>>(system, "gset<float>", true, k);
  test_read<read_k_atom, gset<int>>(system, "gset<float>", true, k);
}

CAF_TEST(read_majority) {
  test_read<read_majority_atom, gset<int>>(system, "gset<int>", false);
  test_read<read_majority_atom, gset<float>>(system, "gset<float>", true);
  test_read<read_majority_atom, gset<int>>(system, "gset<float>", true);
}

CAF_TEST(read_all) {
  test_read<read_all_atom, gset<int>>(system, "gset<int>", false);
  test_read<read_all_atom, gset<float>>(system, "gset<float>", true);
  test_read<read_all_atom, gset<int>>(system, "gset<float>", true);
}

CAF_TEST_FIXTURE_SCOPE_END()
