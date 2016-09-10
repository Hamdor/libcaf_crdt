/******************************************************************************
 *                       ____    _    _____                                   *
 *                      / ___|  / \  |  ___|    C++                           *
 *                     | |     / _ \ | |_       Actor                         *
 *                     | |___ / ___ \|  _|      Framework                     *
 *                      \____/_/   \_|_|                                      *
 *                                                                            *
 * Copyright (C) 2011 - 2016                                                  *
 * Dominik Charousset <dominik.charousset (at) haw-hamburg.de>                *
 * Marian Triebe <marian.triebe (at) haw-hamburg.de>                          *
 *                                                                            *
 * Distributed under the terms and conditions of the BSD 3-Clause License or  *
 * (at your option) under the terms and conditions of the Boost Software      *
 * License 1.0. See accompanying files LICENSE and LICENSE_ALTERNATIVE.       *
 *                                                                            *
 * If you did not receive a copy of the license files, see                    *
 * http://opensource.org/licenses/BSD-3-Clause and                            *
 * http://www.boost.org/LICENSE_1_0.txt.                                      *
 ******************************************************************************/

#ifndef CAF_REPLICATION_REPLICATOR_HPP
#define CAF_REPLICATION_REPLICATOR_HPP

#include "caf/config.hpp"
#include "caf/actor_system.hpp"

#include "caf/replication/replica_actor.hpp"
#include "caf/replication/replicator_actor.hpp"

#include "caf/replication/detail/replicator_hooks.hpp"

namespace caf {
namespace replication {

class replicator : public actor_system::module {
public:
  friend class actor_system;
  replicator(const replicator&) = delete;
  replicator& operator=(const replicator&) = delete;

  void start() override;
  void stop() override;
  void init(actor_system_config&) override;

  id_t id() const override;

  void* subtype_ptr() override;

  static actor_system::module* make(actor_system& sys,
                                    caf::detail::type_list<>);

  replicator_actor actor_handle();

  inline actor_system& system() {
    return system_;
  }

protected:
  replicator(actor_system&);
  ~replicator();

private:
  actor_system& system_;
  replicator_actor manager_;

  // TODO: Make top level replicas spawn at initialisation
  // with actor system config

  std::map<std::string, actor> top_level_replicas_;

  template <class T>
  auto lookup_or_make(const std::string& id)
  -> decltype(system_.spawn(replica_actor<T>, id)) {
    using result_type = decltype(system_.spawn(replica_actor<T>, id));
    auto iter = top_level_replicas_.find(id);
    if (iter != top_level_replicas_.end())
      return actor_cast<result_type>(iter->second);
    // --
    auto act = actor_cast<actor>(system_.spawn(replica_actor<T>, id));
    top_level_replicas_.emplace(id, act);
    return actor_cast<result_type>(act);
  }

public:

  ///
  template <class T>
  void subscribe(std::string id, notifyable_type<T>& subscriber) {
    auto top_level = lookup_or_make<T>(id);
    send_as(subscriber, top_level, subscribe_atom::value, subscriber);
  }

  ///
  template <class T>
  void unsubscribe(std::string id, notifyable_type<T>& subscriber) {
    auto top_level = lookup_or_make<T>(id);
    send_as(subscriber, top_level, unsubscribe_atom::value, subscriber);
  }
};

} // namespace replication
} // namespace caf

#endif // CAF_REPLICATION_REPLICATOR_HPP
