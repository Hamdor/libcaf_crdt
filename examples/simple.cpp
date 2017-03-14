#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "caf/crdt/all.hpp"

using namespace caf;
using namespace caf::io;
using namespace caf::crdt;
using namespace caf::crdt::types;

namespace {

constexpr size_t assumed_entries = 4;

class config : public crdt_config {
public:
  config() {
    add_crdt<types::gset<int>>("gset<int>");
  }
};

void actor_fun(event_based_actor* self) {
  // Initialize state
  auto state_ = std::make_shared<gset<int>>(self, "gset<int>://set");
  self->become(
    [=](int value) {
      state_->insert(value);
    },
    [=](notify_atom, const gset<int>& other) {
      state_->merge(other);
      if (state_->size() == assumed_entries)
        self->quit();
    }
  );
}

void caf_main(actor_system& system, const config&) {
  auto a1 = system.spawn(actor_fun);
  auto a2 = system.spawn(actor_fun);
  auto a3 = system.spawn(actor_fun);
  anon_send(a1, int{1});
  anon_send(a2, int{2});
  anon_send(a3, int{3});
  anon_send(a3, int{4});
}

} // namespace <anonymous>

CAF_MAIN(io::middleman, crdt::replicator)
