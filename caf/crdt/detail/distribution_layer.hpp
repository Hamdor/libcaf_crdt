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
class distribution_layer {

  ///
  struct node_data {
    size_t version;
    replicator_actor replicator;
    std::unordered_set<uri> filter;
  };

  using map_type = std::unordered_map<node_id, node_data>;

public:
  /// Construct a distribution layer
  template <class ReplicatorImpl>
  distribution_layer(ReplicatorImpl* impl)
      : impl_{actor_cast<actor>(impl)},
        local_{0, actor_cast<replicator_actor>(impl), {}} {
    // nop
  }

  /// Add a freshly discovered node
  /// @param nid node to add
  void add_new_node(const node_id& nid) {
    auto& mm = impl_->home_system().middleman();
    auto query = mm.remote_lookup(replicator_atom::value, nid);
    if (query) {
      auto repl = actor_cast<replicator_actor>(query);
      node_data data{0, repl, {}};
      store_.emplace(nid, std::move(data));
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
  /// @param ids      set of ids
  void update(const node_id& nid, size_t version,
              std::unordered_set<uri>&& ids) {
    auto& data = store_[nid];
    if (version > data.version) {
      data.filter = std::move(ids);
      data.version = version;
    }
  }

  /// Locally add a replic Id, this is called, when a new replica<T> is spawned
  /// @param u uri to add
  void add_id(const uri& u) {
    modify_ids(u, false);
  }

  /// Locally remove a replic id
  /// @param u uri to remove
  void remove_id(const uri& u) {
    modify_ids(u, true);
  }

  /// Send the last seen version of a node to all nodes, if there
  /// is a newer version, the node will respond with its updated data.
  void pull_ids() {
    for (auto& entry : store_) {
      auto& data = entry.second;
      send_as(impl_, data.replicator, get_ids_atom::value, data.version);
    }
  }

  /// A node has asked us to respond with our replic Ids, if the version has
  /// changed. Seen is the last seen version by the sender, if we have a newer
  /// version, just respond to intrested node with our actual entry.
  /// @param instested_node the node intrested in our ids
  /// @param seen the last seen version of instrested node
  void get_ids(const node_id& instested_node, size_t seen) {
    if (seen == local_.version)
      return;
    auto iter = store_.find(instested_node);
    if (iter == store_.end())
      return;
    auto& data = iter->second;
    send_as(impl_, data.replicator, local_.version, local_.filter);
  }

  /// Flush updates to intrested remote nodes.
  /// @param id of update as uri
  /// @param msg containing the update
  void publish(const uri& id, const message& msg) const {
    for (auto& entry : store_) {
      auto& set = entry.second.filter;
      if (set.find(id) != set.end())
        send_as(impl_, entry.second.replicator, id, msg);
    }
  }

  std::set<replicator_actor> get_intrested(const uri& id) const {
    std::set<replicator_actor> result;
    for (auto& entry : store_) {
      auto& set = entry.second.filter;
      if (set.find(id) != set.end())
        result.emplace(entry.second.replicator);
    }
    return result;
  }

private:
  /// @private
  inline void modify_ids(const uri& u, bool erase) {
    local_.version++;
    if (erase) local_.filter.erase(u);
    else       local_.filter.emplace(u);
  }

  actor impl_;
  node_data local_;
  map_type store_;
};

} // namespace detail
} // namespace crdt
} // namespace caf

#endif // CAF_CRDT_DETAIL_DISTRIBUTION_LAYER_HPP
