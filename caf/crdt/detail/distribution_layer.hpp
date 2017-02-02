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

#include "caf/crdt/types/gmap.hpp"

#include <unordered_map>
#include <unordered_set>

namespace std {

inline bool operator< (const std::unordered_set<caf::crdt::uri>& lhs,
                       const std::unordered_set<caf::crdt::uri>& rhs) {
  return lhs.size() < rhs.size();
}

template <>
struct hash<std::pair<int, std::unordered_set<caf::crdt::uri>>> {
  size_t operator()(const std::pair<int, std::unordered_set<caf::crdt::uri>>& u) const {
    size_t val = 0;
    for (auto& e : u.second)
      val |= hash<caf::crdt::uri>()(e);
    return val + 73 * u.first;
  }

};

} // namespace std

namespace caf {
namespace crdt {
namespace detail {

/// Manages propagation of updates between nodes. Keeps track which nodes are
/// intrested in topics.
struct distribution_layer {

  /// The payload type is used between nodes to propagate intrested topics
  using payload_type = types::gmap<
    node_id,
    std::pair<size_t, std::unordered_set<uri>
  >>;

  /// Construct a distribution layer
  template <class ReplicatorImpl>
  distribution_layer(ReplicatorImpl* impl) : impl_{actor_cast<actor>(impl)} {
    // nop
  }

  /// Add a freshly discovered node
  /// @param nid node to add
  void add_new_node(const node_id& nid) {
    auto& mm = impl_->home_system().middleman();
    auto query = mm.remote_lookup(replicator_atom::value, nid);
    if (query) {
      auto repl = actor_cast<replicator_actor>(query);
      replicators_.emplace(nid, repl);
      send_as(impl_, repl, get_topics_atom::value);
      // TODO: Send topics and known replicators to new node
    }
  }

  /// A node is no longer reachable, we have to remove it from our lists
  /// @param nid node to remove
  void remove_node(const node_id& nid) {
    topics_.assign(nid, std::make_pair(0, std::unordered_set<uri>{}));
    replicators_.erase(nid);
  }

  /// Add a new topic to a node.
  /// @param nid node which is intrested
  /// @param topic this node is intrested in
  void add_topic(const node_id& nid, const uri& topic) {
    if (nid == impl_->node())
      if (topics_[nid].second.emplace(topic).second) {
        topics_[nid].first++;
        publish(topics_);
      }
  }

  /// Remove a topic from a node.
  /// @param nid related node
  /// @param topic related topic
  void remove_topic(const node_id& nid, const uri& topic) {
    if (nid == impl_->node()) {
      auto& entry = topics_[nid];
      if (entry.second.erase(topic)) {
        topics_[nid].first++;
        publish(topics_);
      }
    }
  }

  ///
  void update_topics(const payload_type& p) {
    for (auto& e : p) {
      auto& key = e.first;
      auto& value = e.second;
      if (topics_[key].first < value.first)
        topics_[key] = value;
    }
  }

  /// Get topics of a specific node.
  /// @param nid local or remote node to lookup topics of
  /// @warning This function only returns the local known view of this node.
  ///          It is possible, that we do not have full knowledge of all topics
  ///          of a remote node.
  std::unordered_set<uri> topics_of(const node_id& nid) const {
    auto iter = topics_.find(nid);
    if (iter == topics_.end())
      return {};
    return iter->second.second;
  }

  /// Flush updates to intrested remote nodes.
  /// @param topic of update as uri
  /// @param msg containing the update
  void publish(const uri& topic, const message& msg) const {
    for (auto& entry : topics_) {
      auto& set = entry.second.second;
      auto iter = set.find(topic);
      if (iter != set.end()) {
        auto repl = replicators_.find(entry.first);
        send_as(impl_, repl->second, topic, msg);
      }
    }
  }

private:

  ///
  void publish(const payload_type& payload) const {
    for (auto& rep : replicators_)
      send_as(impl_, rep.second, update_topics_atom::value, payload);
  }

  types::gmap<node_id, std::pair<size_t, std::unordered_set<uri>>> topics_;
  std::unordered_map<node_id, replicator_actor> replicators_;
  actor impl_;
};

} // namespace detail
} // namespace crdt
} // namespace caf

#endif // CAF_CRDT_DETAIL_DISTRIBUTION_LAYER_HPP
