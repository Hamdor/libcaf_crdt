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

#include "caf/replication/uri.hpp"
#include "caf/replication/notifyable.hpp"
#include "caf/replication/replicator_actor.hpp"

#include "caf/replication/detail/replica.hpp"

namespace caf {
namespace replication {

///
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

  inline actor_system& system() const {
    return system_;
  }

protected:
  replicator(actor_system&);
  ~replicator();

private:
  actor_system& system_;
  replicator_actor manager_;

public:

  ///
  template <class T>
  void subscribe(const uri& u, const notifyable<T>& subscriber) {
    send_as(subscriber, manager_, subscribe_atom::value, u);
  }

  ///
  template <class T>
  void unsubscribe(const uri& u, const notifyable<T>& subscriber) {
    send_as(subscriber, manager_, unsubscribe_atom::value, u);
  }
};

} // namespace replication
} // namespace caf

#endif // CAF_REPLICATION_REPLICATOR_HPP
