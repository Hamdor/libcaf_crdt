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

namespace {

///
template <class T>
struct wr_state {
  size_t messages_left;
  T crdt;
  actor respond_to;
  response_promise rp;
};

///
template <class T>
behavior writer(stateful_actor<wr_state<T>>* self) {
  auto init = [=](size_t msgs_left, actor respond) {
    self->state.messages_left = msgs_left;
    self->state.respond_to = respond;
    self->state.rp = self->make_response_promise();
    if (msgs_left == 0) // This is possible, if write_all was called,
                        // but only one node is known
      self->state.rp.deliver(write_succeed_atom::value);
  };
  return {
    [=](write_all_atom, const uri& u, std::set<replicator_actor> to,
        const message& msg) {
      init(to.size(), actor_cast<actor>(self->current_sender()));
      for (auto& rep : to)
        self->send(rep, write_local_atom::value, u, msg);
    },
    [=](write_majority_atom, const uri& u, std::set<replicator_actor> to,
        const message& msg) {
      auto n = to.size() / 2 + 1;
      init(n, actor_cast<actor>(self->current_sender()));
      size_t send = 0;
      for (auto& rep : to) {
        self->send(rep, write_local_atom::value, u, msg);
        if (++send == n)
          break;
      }
    },
    [=](write_succeed_atom) {
      if (--self->state.messages_left == 0)
        self->state.rp.deliver(write_succeed_atom::value);
    }
  };
}

///
template <class T>
behavior reader(stateful_actor<wr_state<T>>* self) {
  auto init = [=](size_t msgs_left, actor respond) {
    self->state.messages_left = msgs_left;
    self->state.respond_to = respond;
    self->state.rp = self->make_response_promise();;
  };
  return {
    [=](read_all_atom, const uri& u, std::set<replicator_actor> from) {
      init(from.size(), actor_cast<actor>(self->current_sender()));
      for (auto& rep : from)
        self->send(rep, read_local_atom::value, u);
    },
    [=](read_majority_atom, const uri& u, std::set<replicator_actor> from) {
      auto n = from.size() / 2 + 1;
      init(n, actor_cast<actor>(self->current_sender()));
      size_t send = 0;
      for (auto& rep : from) {
        self->send(rep, read_local_atom::value, u);
        if (++send == n)
          break;
      }
    },
    [=](read_succeed_atom, const T& msg) {
      self->state.crdt.merge(msg);
      if (--self->state.messages_left == 0)
        self->state.rp.deliver(read_succeed_atom::value, self->state.crdt);
    }
  };
}

} // namespace <anonymous>

///
template <class T>
struct replica : public event_based_actor {
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
      },
      [&](read_all_atom, const uri& u, std::set<replicator_actor> from) {
        from.emplace(this->system().replicator().actor_handle());
        delegate(this->spawn(reader<T>), read_all_atom::value, u,
                 std::move(from));
      },
      [&](read_majority_atom, const uri& u, std::set<replicator_actor> from) {
        delegate(this->spawn(reader<T>), read_majority_atom::value, u,
                 std::move(from));
      },
      [&](read_local_atom) -> result<read_succeed_atom, T> {
        return {read_succeed_atom::value, cvrdt_};
      },
      [&](write_all_atom, const uri& u, std::set<replicator_actor> to,
          const message& msg) {
        send(this, publish_atom::value, msg);
        delegate(this->spawn(writer<T>), write_all_atom::value, u, std::move(to),
                 msg);
      },
      [&](write_majority_atom, const uri& u, std::set<replicator_actor> to,
          const message& msg) {
        send(this, publish_atom::value, msg);
        delegate(this->spawn(writer<T>), write_majority_atom::value, u,
                 std::move(to), msg);
      },
      [&](write_local_atom, message& msg) { // Write the content of msg to our state
        send(this, publish_atom::value, msg);
        return write_succeed_atom::value;
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
