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

#include "caf/replication/replicator_actor.hpp"

#include "caf/message.hpp"
#include "caf/node_id.hpp"
#include "caf/typed_event_based_actor.hpp"

#include "caf/io/middleman.hpp"

#include "caf/replication/detail/distribution_layer.hpp"

#include <tuple>
#include <vector>
#include <unordered_map>

namespace caf {
namespace replication {

namespace {

/// Implementation of replicator actor
struct replicator_actor_impl : public replicator_actor::base {

  replicator_actor_impl(actor_config& cfg)
      : replicator_actor::base(cfg), dist(this) {
    // nop
  }

  virtual ~replicator_actor_impl() {
    for (auto& r : replicas_)
      anon_send_exit(r.second, exit_reason::user_shutdown);
    replicas_.clear();
  }

  const char* name() const override {
    return "replicator_actor";
  }

protected:
  behavior_type make_behavior() override {
    send(this, tick_atom::value);
    return {
      [&](const uri& topic, message& msg) {
        if(current_sender()->node() == this->node())
          updates_.emplace_back(std::make_tuple(topic, std::move(msg)));
        else {
          auto iter = replicas_.find(topic);
          if (iter != replicas_.end())
            anon_send(iter->second, publish_atom::value, std::move(msg));
        }
      },
      [&](tick_atom) {
        for(auto& update : updates_) {
          auto& topic = std::get<0>(update);
          auto& payload = std::get<1>(update);
          dist.publish(topic, payload);
        }
        updates_.clear();
        delayed_send(this, std::chrono::milliseconds(250), tick_atom::value);
      },
      [&](new_connection_atom, const node_id& node) {
        dist.add_new_node(node);
      },
      [&](connection_lost_atom, const node_id& nid) {
        dist.remove_node(nid);
      },
      [&](get_topics_atom) {
        return dist.topics_of(node());
      },
      [&](add_topic_atom, const uri& topic) {
        dist.add_topic(current_sender()->node(), topic);
      },
      [&](remove_topic_atom, const uri& topic) {
        dist.remove_topic(current_sender()->node(), topic);
      },
      [&](update_topics_atom, detail::distribution_layer::payload_type& p) {
        dist.update_topics(std::move(p));
      },
      [&](subscribe_atom, const uri& u) {
        auto iter = replicas_.find(u);
        if (iter == replicas_.end()) {
          auto opt = system().spawn<actor>(u.scheme(),
                                           make_message(u.to_string()));
          if (opt) {
            iter = replicas_.emplace(u, *opt).first;
            dist.add_topic(node(), u);
          } else
            return; // TODO: Handle error -> error<...>
        }
        anon_send(iter->second, subscribe_atom::value,
                  actor_cast<actor>(current_sender()));
      },
      [&](unsubscribe_atom, const uri& u) {
        auto iter = replicas_.find(u);
        if (iter == replicas_.end())
          return;
        anon_send(iter->second, unsubscribe_atom::value,
                  actor_cast<actor>(current_sender()));
      }
    };
  }

private:
  /// Update buffer
  std::vector<std::tuple<uri, message>> updates_;
  /// Map of local replicas
  std::unordered_map<uri, actor> replicas_;
  /// Organize distribution of updates
  detail::distribution_layer dist;
};

} // namespace <anonymous>

replicator_actor make_replicator_actor(actor_system& sys) {
  return sys.spawn<replicator_actor_impl, hidden>();
}

} // namespace replication
} // namespace caf
