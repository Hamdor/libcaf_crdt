#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "caf/crdt/all.hpp"

#include <unordered_set>
#include <iostream>

using namespace caf;
using namespace caf::io;
using namespace caf::crdt;
using namespace caf::crdt::types;

namespace {

static uri u{"gcounter<int>://counter"};

constexpr int actors  = 10;
constexpr int to_add  = 4;
constexpr int assumed = actors * to_add;

class config : public crdt_config {
public:
  config() {
    add_crdt<types::gcounter<int>>("gcounter<int>");
  }
};

class incrementer : public gcounter<int>::base {
public:
  incrementer(actor_config& cfg) : gcounter<int>::base(cfg) {
    system().replicator().subscribe<gcounter<int>>(u, this);
  }

  virtual void on_exit() override {
    //system().replicator().unsubscribe<gcounter<int>>(u, this);
    gcounter<int>::base::on_exit();
  }

protected:
  typename gcounter<int>::behavior_type make_behavior() override {
    return {
      [&](initial_atom, gcounter<int>& state) {
        state_ = std::move(state);
        state_ += to_add;
      },
      [&](notify_atom, const typename gcounter<int>::transaction_t& t) {
        state_.apply(t);
        if (state_.count() == assumed)
          quit();
      }
    };
  }

private:
  gcounter<int> state_;
};

void caf_main(actor_system& system, const config&) {
  for (int i = 0; i < actors; ++i)
    system.spawn<incrementer>();
}

} // namespace <anonymous>

CAF_MAIN(io::middleman, crdt::replicator)
