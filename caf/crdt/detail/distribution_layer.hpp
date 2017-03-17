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

#ifndef CAF_CRDT_DETAIL_DISTRIBUTION_LAYER_HPP
#define CAF_CRDT_DETAIL_DISTRIBUTION_LAYER_HPP

#include "caf/node_id.hpp"

#include "caf/crdt/uri.hpp"

#include <tuple>
#include <unordered_map>
#include <unordered_set>

namespace caf {
namespace crdt {
namespace detail {

/// Manages the data send between nodes.
struct distribution_layer {

  using tuple_type = std::tuple<size_t, replicator_actor,
                                std::unordered_set<uri>>;
  using map_type   = std::unordered_map<node_id, tuple_type>;

  /// Construct a distribution layer
  template <class ReplicatorImpl>
  distribution_layer(ReplicatorImpl* impl) : impl_{actor_cast<actor>(impl)} {
    auto tup = std::make_tuple(0, actor_cast<replicator_actor>(impl),
                               std::unordered_set<uri>{});
    store_.emplace(impl_->node(), std::move(tup));
  }

  /// Add a freshly discovered node
  /// @param nid node to add
  void add_new_node(const node_id& nid) {
    auto& mm = impl_->home_system().middleman();
    auto query = mm.remote_lookup(replicator_atom::value, nid);
    if (query) {
      auto repl = actor_cast<replicator_actor>(query);
      store_.emplace(nid, std::make_tuple(0, repl, std::unordered_set<uri>{}));
      send_as(impl_, repl, get_ids_atom::value, size_t{0});
    }
  }

  /// A node is no longer reachable, we have to remove it from our lists
  /// @param nid node to remove
  void remove_node(const node_id& nid) {
    store_.erase(nid);
  }

  /// Update a map entry
  /// @param nid      regarding node
  /// @param version  version of set
  /// @param topics   set of topics
  void update(const node_id& nid, size_t version,
              std::unordered_set<uri>&& topics) {
    auto& tup  = store_[nid];
    auto& ver  = std::get<0>(tup);
    auto& set  = std::get<2>(tup);
    if (version > ver) {
      set = std::move(topics);
      ver = version;
    }
  }

  /// Locally add a topic, this is called, when a new replica<T> is spawned
  /// @param u uri to add
  void add_topic(const uri& u) {
    modify_topics(u, false);
  }

  /// Locally remove a topic
  /// @param u uri to remove
  void remove_topic(const uri& u) {
    modify_topics(u, true);
  }

  /// Send the last seen version of a node to the node, if there
  /// is a newer version, the node will respond with its updated data.
  void pull_topics() {
    for (auto& entry : store_) {
      if (entry.first == impl_->node())
        continue;
      auto& seen = std::get<0>(entry.second);
      auto& rep  = std::get<1>(entry.second);
      send_as(impl_, rep, get_ids_atom::value, seen);
    }
  }

  /// A node has asked us to respond with our topics, if the version has
  /// changed. Seen is the last seen version by the sender, if we have a newer
  /// version, just respond to intrested node with our actual entry.
  /// @param instested_node the node intrested in our topics
  /// @param seen the last seen version of instrested node
  void get_topics(const node_id& instested_node, size_t seen) {
    auto& tup = store_[impl_->node()];
    if (seen < std::get<0>(tup)) {
      auto iter = store_.find(instested_node);
      if (iter == store_.end())
        return;
      send_as(impl_, std::get<1>(iter->second), std::get<0>(tup),
              std::get<2>(tup));
    }
  }

  /// Flush updates to intrested remote nodes.
  /// @param topic of update as uri
  /// @param msg containing the update
  void publish(const uri& topic, const message& msg) const {
    for (auto& entry : store_) {
      if (entry.first == impl_->node())
        continue;
      auto& set = std::get<2>(entry.second);
      auto iter = set.find(topic);
      if (iter != set.end()) {
        auto& repl = std::get<1>(entry.second);
        send_as(impl_, repl, topic, msg);
      }
    }
  }

  std::set<replicator_actor> get_intrested(const uri& topic) const {
    std::set<replicator_actor> result;
    for (auto& entry : store_) {
      if (entry.first == impl_->node())
        continue;
      auto& set = std::get<2>(entry.second);
      auto iter = set.find(topic);
      if (iter != set.end())
        result.emplace(std::get<1>(entry.second));
    }
    return result;
  }

private:

  inline void modify_topics(const uri& u, bool erase) {
    auto& tup = store_[impl_->node()];
    auto& ver = std::get<0>(tup);
    ver++;
    auto& set = std::get<2>(tup);
    if (erase) set.erase(u);
    else       set.emplace(u);
  }

  map_type store_;
  actor impl_;
};

} // namespace detail
} // namespace crdt
} // namespace caf

#endif // CAF_CRDT_DETAIL_DISTRIBUTION_LAYER_HPP
