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

#ifndef CAF_CRDT_DETAIL_REPLICA_HPP
#define CAF_CRDT_DETAIL_REPLICA_HPP

#include "caf/event_based_actor.hpp"

#include "caf/crdt/uri.hpp"
#include "caf/crdt/atom_types.hpp"
#include "caf/crdt/notifyable.hpp"
#include "caf/crdt/replicator_actor.hpp"

#include <unordered_set>

namespace caf {
namespace crdt {
namespace detail {

///
template <class T>
class replica : public event_based_actor {
public:
  replica(actor_config& cfg, const uri& topic)
      : event_based_actor(cfg), topic_{topic} {
    // nop
  }

protected:
  virtual behavior make_behavior() override {
    return {
      [&](publish_atom, message& msg) {
        T unpacked;
        msg.apply([&](T& t) {
          unpacked = std::move(t);
        });
        auto delta = cvrdt_.merge(unpacked);
        if (delta.empty())
          return; // State was already included
        for (auto& sub : subs_)
          if (sub != current_sender())
            send(sub, notify_atom::value, unpacked);
      },
      [&](subscribe_atom) {
        auto handle = actor_cast<actor>(current_sender());
        subs_.emplace(handle);
        // Send current full state to subscriber
        if (!cvrdt_.empty())
          send(handle, notify_atom::value, cvrdt_);
      },
      [&](unsubscribe_atom) {
        subs_.erase(actor_cast<actor>(current_sender()));
      },
      [&](copy_atom) {
        send(system().replicator().actor_handle(),
             copy_ack_atom::value, topic_, make_message(cvrdt_));
      }
    };
  }

private:
  T cvrdt_;                        /// CRDT State (complete state)
  uri topic_;                      /// Topic
  std::unordered_set<actor> subs_; /// Subscribers
};

} // namespace detail
} // namespace crdt
} // namespace caf

#endif // CAF_CRDT_DETAIL_REPLICA_HPP
