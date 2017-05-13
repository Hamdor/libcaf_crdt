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

struct state {
  state(event_based_actor* self) : crdt(self, "gset<int>://set") {
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
      if (self->state.crdt.size() == assumed_entries) {
        aout(self) << "got all entries... exiting...\n";
        self->quit();
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
  std::this_thread::sleep_for(std::chrono::seconds(5));
}

} // namespace <anonymous>

CAF_MAIN(io::middleman, crdt::replicator)
