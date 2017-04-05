#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "caf/crdt/all.hpp"

using namespace caf;
using namespace caf::io;
using namespace caf::crdt;
using namespace caf::crdt::types;

namespace {

constexpr size_t assumed_entries = 4;
constexpr size_t to_spawn        = 4;

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
      if (state_->size() == assumed_entries) {
        self->quit();
        aout(self) << "got all entries... exiting...\n";
      }
    }
  );
}

void caf_main(actor_system& system, const config&) {
  std::vector<int> v(to_spawn);
  int val = 0;
  std::generate(v.begin(), v.end(), [&] { return val++; });
  for (size_t i = 0; i < to_spawn; ++i)
    anon_send(system.spawn(actor_fun), v[i]);
}

} // namespace <anonymous>

CAF_MAIN(io::middleman, crdt::replicator)
