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
  uri u {"gset<int>://test"};
  auto repl = actor_cast<actor>(system.replicator().actor_handle());
  scoped_actor self{system};
  gset<int> set(self, "gset<int>://test");
  set.insert(5);
  auto req1 = self->request(repl, infinite, write_all_atom::value, u,
                            make_message(set));
  req1.receive(
    [&](write_succeed_atom) {
      std::cout << "succeed..." << std::endl;
    },
    [&](error&) {
      std::cout << "error..." << std::endl;
    }
  );
  auto req2 = self->request(repl, infinite, read_all_atom::value, u);
  req2.receive(
    [&](read_succeed_atom, const gset<int>& result) {
      std::cout << "succeed..." << std::endl
                << "contains 5? " << (result.element_of(5) ? "yes" : "no")
                << std::endl;
    },
    [&](error&) {
      std::cout << "error..." << std::endl;
    }
  );
}

} // namespace <anonymous>

CAF_MAIN(io::middleman, crdt::replicator)