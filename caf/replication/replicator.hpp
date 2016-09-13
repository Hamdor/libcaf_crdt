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

#include "caf/replication/replicator_actor.hpp"

#include "caf/replication/detail/replica_actor.hpp"
#include "caf/replication/detail/replicator_hooks.hpp"
#include "caf/replication/detail/root_replica_actor.hpp"

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

  std::map<std::string, actor> root_level_replicas_;

  std::vector<std::tuple<std::string, std::string, actor>> replicas_;

  template <class T>
  void make_test_tree(const std::string& topic) {
    static bool init = false;
    if (init != false) return;
    init = true;
    // Spawn a new tree:
    //       root
    //      /   \
    //   sub1    sub2
    //   /
    // subsub1
    // --- Spawn actors
    auto root = system_.spawn(detail::root_replica_actor<T>, topic);
    auto sub1 = system_.spawn(detail::replica_actor<T>, topic);
    auto sub2 = system_.spawn(detail::replica_actor<T>, topic);
    auto subsub1 = system_.spawn(detail::replica_actor<T>, topic);
    // --- Build tree (set partens)
    anon_send(subsub1, set_parent_atom::value, actor_cast<publishable_t<T>>(sub1));
    anon_send(sub1, set_parent_atom::value, actor_cast<publishable_t<T>>(root));
    anon_send(sub2, set_parent_atom::value, actor_cast<publishable_t<T>>(root));
    // --- Build tree (set childs)
    anon_send(sub1, add_child_atom::value, actor_cast<publishable_t<T>>(subsub1));
    anon_send(root, add_child_atom::value, actor_cast<publishable_t<T>>(sub1));
    anon_send(root, add_child_atom::value, actor_cast<publishable_t<T>>(sub2));
    // --- Add to maps
    root_level_replicas_.emplace(topic, actor_cast<actor>(root));
    replicas_.emplace_back(std::make_tuple(topic, std::string{"/sub1"}, actor_cast<actor>(sub1)));
    replicas_.emplace_back(std::make_tuple(topic, std::string{"/sub1/subsub1"}, actor_cast<actor>(subsub1)));
    replicas_.emplace_back(std::make_tuple(topic, std::string{"/sub2"}, actor_cast<actor>(sub2)));
  }

  template <class T>
  subscribable_t<T> lookup_or_make(const std::string& topic,
                                   const std::string& path) {
    make_test_tree<T>(topic);
    actor res{unsafe_actor_handle_init};
    for (auto& tuple : replicas_)
      if (std::get<0>(tuple) == topic &&
          std::get<1>(tuple) == path)
          res = std::get<2>(tuple);
    return actor_cast<subscribable_t<T>>(res);
  }

public:

  ///
  template <class T>
  void subscribe(const std::string& id, const std::string& path,
                 notifyable_type<T>& subscriber) {
    auto replica = lookup_or_make<T>(id, path);
    send_as(subscriber, replica, subscribe_atom::value, subscriber);
}

  ///
  template <class T>
  void unsubscribe(const std::string& id, const std::string& path,
                   notifyable_type<T>& subscriber) {
    auto replica = lookup_or_make<T>(id, path);
    send_as(subscriber, replica, unsubscribe_atom::value, subscriber);
  }
};

} // namespace replication
} // namespace caf

#endif // CAF_REPLICATION_REPLICATOR_HPP
