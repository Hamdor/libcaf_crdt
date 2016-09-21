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

#ifndef CAF_REPLICATION_DETAIL_REPLICA_MAP_HPP
#define CAF_REPLICATION_DETAIL_REPLICA_MAP_HPP

#include "caf/node_id.hpp"

#include "caf/replication/crdt/base_datatype.hpp"
#include "caf/replication/crdt/base_transaction.hpp"

#include "caf/replication/crdt/gmap.hpp"
#include "caf/replication/crdt/gset.hpp"

namespace caf {
namespace replication {
namespace detail {

namespace {

/// Transactions supportet by the replica_map
enum class replica_map_operation {
  none,
  write
};

/// Describes a transaction for replica_map as CmRDT
struct replica_map_transaction : public crdt::base_transaction {
  using operation_t = replica_map_operation;

  using set_type = std::set<node_id>;
  using map_type = std::map<std::string, set_type>;

  ///
  replica_map_transaction(std::string topic, actor owner, operation_t op,
                          map_type map)
      : base_transaction(std::move(topic), std::move(owner)),
        op_(std::move(op)), map_(std::move(map)) {
    // nop
  }

  ///
  replica_map_transaction(std::string topic, actor owner, operation_t op,
                          std::string key, node_id nid)
      : base_transaction(std::move(topic), std::move(owner)),
        op_(std::move(op)) {
    map_.emplace(std::move(key), std::set<node_id>{std::move(nid)});
  }

  virtual ~replica_map_transaction() = default;

  ///
  inline const operation_t& operation() const { return op_; }

  ///
  inline const map_type& map() const { return map_; }

  ///
  inline const set_type& set(const std::string& key) const {
    return map_.at(key);
  }

  ///
  inline std::set<std::string> key_set() const {
    std::set<std::string> keys;
    auto iter = map_.begin();
    while (map_.end() != iter)
      keys.insert((iter++)->first);
    return keys;
  }

  template <class Processor>
  friend void serialize(Processor& proc, replica_map_transaction& x) {
    proc & x.op_;
    proc & x.map_;
  }

private:
  operation_t op_;
  std::map<std::string, std::set<node_id>> map_;
};

namespace delta {

///
struct replica_map_impl {
  using operator_t = replica_map_operation;
  using transaction_t = replica_map_transaction;

  using set_type = crdt::gset<node_id>;
  using map_type = crdt::gmap<std::string, set_type>;

  replica_map_impl() = default;

  replica_map_impl(map_type map)
      : map_(std::move(map)) {
    // nop
  }

  /// Apply transaction to map
  /// @param history a transaction
  /// @return a delta-CRDT which represent the delta
  replica_map_impl apply(const transaction_t& history) {
    map_type delta;
    if (history.operation() == operator_t::none)
      return {};
    for (auto& key : history.key_set()) {
      auto& values = history.set(key);
      // const_cast is valid here, we will only have an increasing state
      auto& set = const_cast<set_type&>(map_.at(key));
      auto t = set.subset_insert(values);
      // Add inserted values to delta
      set = const_cast<set_type&>(delta.at(key));
      set.subset_insert(t.values());
    }

    return {std::move(delta)};
  }

  /// Merge two delta-CRDTs
  replica_map_impl merge(const std::string& topic,
                         const replica_map_impl& other) {
    map_type delta;
    for (auto& pair : other.map_.map()) {
      auto iter = map_.map().find(pair.first);
      if (iter == map_.map().end()) {
        if (map_.assign(pair.first, pair.second))
          // Add to delta
          delta.assign(pair.first, pair.second);
      } else {
        // Merge both
        auto& set = const_cast<set_type&>(map_.at(pair.first));
        auto t = set.subset_insert(pair.second.get_immutable());
        delta.assign(pair.first, {t.values()});
      }
    }
    return {std::move(delta)};
  }

  ///
  transaction_t get_cmrdt_transactions(const std::string& topic,
                                       const actor& creator) const {
    std::map<std::string, std::set<node_id>> map;
    for (auto& e : map_.map())
      map.emplace(e.first, e.second.get_immutable());
    return {topic, creator, operator_t::write, map};
  }

private:
  map_type map_;
};

} // namespace delta

namespace cmrdt {

///
class replica_map_impl : public crdt::base_datatype {
  using set_type = crdt::gset<node_id>;
  using map_type = crdt::gmap<std::string, set_type>;
public:
  using operator_t = replica_map_operation;
  using transaction_t = replica_map_transaction;

  /// Add a single topic to a node
  /// @param topic used as key
  /// @param nid node_id to add
  bool add_topic(const std::string& topic, const node_id& nid) {
    auto set = const_cast<set_type&>(map_.at(topic));
    auto t = set.insert(nid);
    if (t.operation() != set_type::operator_t::none) {
      publish(transaction_t{t.topic(), t.creator(), operator_t::write,
                            topic, nid});
      return true;
    }
    return false;
  }

  /// Add multiple topics to a node
  void add_topics(const std::set<std::string>& topics, const node_id& nid) {
    for (const auto& topic : topics)
      add_topic(topic, nid);
  }

  /// Remove node
  /// @param nid node to remove
  /// @returns `true` if node was removed
  ///          `false` otherwise
  bool remove_node(const node_id& nid) {
    // TODO: Search in all topics for nid and remove it
    // TODO: Remove from CvRDT version also!
    // DO NOT GENERATE A UPDATE!
  }

  ///
  const std::set<node_id>& lookup(const std::string& topic) {
    return map_.at(topic).get_immutable();
  }

private:
  map_type map_;
};

} // namespace cmrdt

} // namespace <anonymous>

///
struct replica_map : public cmrdt::replica_map_impl {
  ///
  using internal_t = delta::replica_map_impl;
  ///
  using transaction_t = typename cmrdt::replica_map_impl::transaction_t;
};

} // namespace detail
} // namespace replication
} // namespace caf


#endif // CAF_REPLICATION_DETAIL_REPLICA_MAP_HPP
