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
      : event_based_actor(cfg), topic_{std::move(topic)} {
    // nop
  }

  virtual void on_exit() override {
    event_based_actor::on_exit();
  }

protected:
  behavior make_behavior() override {
    this->send(this, tick_atom::value);
    return {
      [&](publish_atom, message& msg) {
        typename T::internal_t delta;
        auto unpack = [&](typename T::internal_t& unpacked) {
          delta = std::move(unpacked);
        };
        msg.apply(unpack);
        if (delta.empty())
          return;
        delta = cvrdt_.merge(delta);
        if (delta.empty())
          return;
        auto ops = delta.get_cmrdt_transactions(topic_.str());
        for (auto& subs : subscribers_)
          this->send(subs, notify_atom::value, ops);
      },
      [&](publish_atom, const typename T::transaction_t& transaction) {
        auto delta = cvrdt_.apply(transaction, this->node());
        if (delta.empty())
          return;
        buffer_.merge(delta);
        for (auto& sub : subscribers_)
          if (sub != this->current_sender())
            this->send(sub, notify_atom::value, transaction);
      },
      [&](subscribe_atom, const actor& subscriber) {
        subscribers_.emplace(subscriber);
        T state;
        state.apply(cvrdt_.get_cmrdt_transactions(topic_.str()));
        state.set_owner(subscriber);
        state.set_parent(this);
        state.set_topic(topic_.str());
        this->send(subscriber, initial_atom::value, std::move(state));
      },
      [&](unsubscribe_atom, const actor& subscriber) {
        subscribers_.erase(subscriber);
      },
      [&](tick_atom) {
        this->send(this->home_system().replicator().actor_handle(), topic_,
                   count_++ % 10 ? make_message(cvrdt_) : make_message(buffer_));
        this->delayed_send(this, std::chrono::seconds(1), tick_atom::value);
        buffer_.clear();
      }
    };
  }

private:
  size_t count_;
  uri topic_;
  typename T::internal_t cvrdt_;    /// CvRDT state
  typename T::internal_t buffer_;   /// Delta buffer
  std::unordered_set<actor> subscribers_;
};

} // namespace detail
} // namespace crdt
} // namespace caf

#endif // CAF_CRDT_DETAIL_REPLICA_HPP
