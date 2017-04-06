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

#define CAF_SUITE gset
#include "caf/test/unit_test.hpp"

#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "caf/crdt/all.hpp"
/*
using namespace caf;
using namespace caf::crdt;

namespace {

constexpr int buddy_count = 5;
constexpr int runs        = 10;

constexpr int assumed_msgs = buddy_count * runs;

uri u{"gset<int>://rand"};

using request_atom = atom_constant<atom("req")>;
using verify_atom = atom_constant<atom("verify")>;
using checked_atom = atom_constant<atom("checked")>;

using testee_type = typed_actor<
  replies_to<tick_atom>::with<int>,
  replies_to<request_atom>::with<types::gset<int>>
>::extend_with<notifiable<types::gset<int>>>;

struct testee : public testee_type::base {
  testee(actor_config& cfg)
    : testee_type::base(cfg) {
    system().replicator().subscribe<types::gset<int>>(u, this);
  }

protected:
  typename testee_type::behavior_type make_behavior() override {
    return {
      [&](initial_atom, types::gset<int>& state) {
        state_ = std::move(state);
      },
      [&](notify_atom, const types::gset<int>::transaction_t& operations) {
        state_.apply(operations);
      },
      // --- Generate new data
      [&](tick_atom) {
        auto rnd = std::rand();
        state_.insert(rnd);
        return rnd;
      },
      [&](request_atom) {
        return state_;
      }
    };
  }

private:
  types::gset<int> state_;
};

struct verifier : public event_based_actor {
  verifier(actor_config& cfg) : event_based_actor(cfg) {
    // nop
  }

protected:
  virtual behavior make_behavior() override {
    // Spawn testees
    for (int i = 0; i < buddy_count; ++i)
      buddies_.emplace(spawn<testee>());
    // Send initial tasks
    for (int i = 0; i < runs; ++i)
      for (auto& buddy : buddies_)
        send(buddy, tick_atom::value);
    return {
      // A testee inserted a value n
      [&](int n) {
        values_.insert(n);
        // Let some time pass to give everyone the chance to apply pending
        // updates
        if (++msgs == assumed_msgs)
          delayed_send(this, std::chrono::seconds(2), verify_atom::value);
      },
      // trigger testees to send their state back
      [&](verify_atom) {
        for (auto& buddy : buddies_)
          send(buddy, request_atom::value);
      },
      // check testees states if they are equal
      [&](const types::gset<int>& check) {
        CAF_CHECK(check.equal(values_));
        send(this, checked_atom::value);
      },
      // if all are checked, just exit all actors...
      [&](checked_atom) {
        if (++checked_ == buddy_count) {
          for (auto& buddy : buddies_)
            anon_send_exit(buddy, exit_reason::user_shutdown);
          quit();
        }
      }
    };
  }

private:
  int checked_ = 0;
  int msgs = 0;
  std::set<int> values_;
  std::set<testee_type> buddies_;
};

void test_init(const std::set<int>& init_val,
               const std::set<int>& def_val,
               const std::set<int>& assumed) {
  types::gset<int> init{std::move(init_val)};
  types::gset<int> def{std::move(def_val)};
  def = std::move(init);
  for (auto& e : assumed)
    CAF_CHECK(def.element_of(e));
}

} // namespace <anonymous>

// This test let some actors work with the replica data
CAF_TEST(run_inserts) {
  crdt_config cfg{};
  cfg.add_crdt<types::gset<int>>("gset<int>")
     .add_message_type<std::set<int>>("set_int")
     .load<io::middleman>();
  actor_system system{cfg};
  system.spawn<verifier>();
}

// Tests the initialization process of crdt instances
CAF_TEST(move1) {
  test_init({1},{2},{1,2});
}

// Tests the initialization process of crdt instances
CAF_TEST(move2) {
  test_init({1},{},{1});
}

// Tests the initialization process of crdt instances
CAF_TEST(move3) {
  test_init({},{1},{1});
}*/
