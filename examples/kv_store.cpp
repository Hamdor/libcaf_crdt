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
    add_crdt<gmap<std::string, int>>("map<string,int>");
  }
};

template <class Key, class Value>
using provider_actor_type = typename typed_actor<
  typename replies_to<Key, Value>::template with<bool>,
  typename replies_to<Key>::template with<Value>
>::template extend_with<
  notifiable<gmap<Key,Value>>
>;

template <class Key, class Value>
class provider_actor : public provider_actor_type<Key, Value>::base {
  using typed_base = provider_actor_type<Key, Value>;
  using key_type = Key;
  using value_type = Value;
  using map_type = gmap<key_type, value_type>;
public:
  provider_actor(actor_config& cfg) :
    typed_base::base(cfg), map_{this, "map<string,int>://kv"} {
    // nop
  }

protected:
  typename typed_base::behavior_type make_behavior() override {
    return {
      [&](const key_type& key, const value_type& value) -> bool {
        return map_.set(key, value);
      },
      [&](const key_type& key) -> optional<value_type> {
        return map_.get(key);
      },
      [&](notify_atom, const map_type& other) {
        map_.merge(other);
      }
    };
  }

private:
  map_type map_;
};

void caf_main(actor_system& system, const config&) {
  auto provider = system.spawn<provider_actor<std::string, int>>();
  scoped_actor self{system};
  auto set_result_handler = [&](bool result) {
    aout(self) << (result ? "value was written\n" : "value was not written\n");
  };
  auto get_result_handler = [&](int value) {
    aout(self) << "number of students: " << value << "\n";
  };
  auto error_handler = [&](error) {};
  std::string key = "num_students";
  self->request(provider, infinite, key, 12).receive(set_result_handler,
                                                     error_handler);
  self->request(provider, infinite, key, 11).receive(set_result_handler,
                                                     error_handler);
  self->request(provider, infinite, key).receive(get_result_handler,
                                                 error_handler);
  gmap<std::string, int> map{self};
  auto req = self->request(actor_cast<actor>(system.replicator().actor_handle()), infinite,
                           read_local_atom::value, uri{"map<string,int>://kv"});
  req.receive(
    [&](read_succeed_atom, const gmap<std::string, int>& result) {
      std::cout << "Read: map[" << key << "] = " << *result.get(key)
                << std::endl;
    },
    error_handler
  );
  destroy(provider);
  destroy(self);
}

} // namespace <anonymous>

CAF_MAIN(io::middleman, crdt::replicator)
