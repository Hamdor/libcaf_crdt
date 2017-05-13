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

#ifndef CAF_CRDT_DETAIL_ABSTRACT_DISTRIBUTION_LAYER_HPP
#define CAF_CRDT_DETAIL_ABSTRACT_DISTRIBUTION_LAYER_HPP

#include "caf/fwd.hpp"

#include "caf/crdt/uri.hpp"

namespace caf {
namespace crdt {
namespace detail {

/// Manages the data send between nodes.
class abstract_distribution_layer {
public:
  /// Add a freshly discovered node
  /// @param nid node to add
  virtual void add_new_node(const node_id& nid) = 0;

  /// A node is no longer reachable, we have to remove it from our lists
  /// @param nid node to remove
  virtual void remove_node(const node_id& nid) = 0;

  /// Update a map entry
  /// @param nid      regarding node
  /// @param version  version of set
  /// @param ids      set of ids
  virtual void update(const node_id& nid, size_t version,
                      std::unordered_set<uri>&& ids) = 0;

  /// Locally add a replic Id, this is called, when a new replica<T> is spawned
  /// @param u uri to add
  virtual void add_id(const uri& u) = 0;

  /// Locally remove a replic id
  /// @param u uri to remove
  virtual void remove_id(const uri& u) = 0;

  /// Add update into update buffer
  /// @param id of update as uri
  /// @param msg containing the update
  virtual void publish(const uri& id, const message& msg) = 0;

  /// Flushes the update buffer
  virtual void flush_buffer() = 0;

  /// Get intrested nodes to a id
  /// @param id replic id
  /// @returns set of replicator_actors
  virtual std::set<replicator_actor> get_intrested(const uri& id) const = 0;
};

} // namespace detail
} // namespace crdt
} // namespace caf

#endif // CAF_CRDT_DETAIL_ABSTRACT_DISTRIBUTION_LAYER_HPP
