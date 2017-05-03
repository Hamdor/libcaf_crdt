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

#ifndef CAF_CRDT_TYPES_GSET_HPP
#define CAF_CRDT_TYPES_GSET_HPP

#include <set>
#include <algorithm>

#include "caf/node_id.hpp"

#include "caf/detail/comparable.hpp"

#include "caf/crdt/types/base_datatype.hpp"

namespace caf {
namespace crdt {
namespace types {

/// GSet implementation as delta-CRDT
template <class T>
class gset : public base_datatype, caf::detail::comparable<gset<T>> {

  gset(const T& elem) : set_{elem} {
    // nop
  }

  gset(std::set<T> set) : set_(std::move(set)) {
    // nop
  }

public:
  using value_type = T;
  using interface = notifiable<gset<T>>;
  using base = typename notifiable<gset<T>>::base;
  using behavior_type = typename notifiable<gset<T>>::behavior_type;

  gset() = default;

  template <class ActorType>
  gset(ActorType&& owner, std::string id)
    : base_datatype(std::forward<ActorType>(owner), std::move(id)) {
    // nop
  }

  /// Merge function, for this type it is simple
  /// @param other delta-CRDT to merge into this
  /// @returns a delta gset<T>
  gset<T> merge(const gset<T>& other) {
    std::set<T> delta;
    for (auto& elem : other.set_)
      if (internal_emplace(elem))
        delta.emplace(elem);
    return {std::move(delta)};
  }

  /// Insert a element into this gset
  /// @param elem to insert
  /// @return `true`  if elem is inserted
  ///         `false` if elem is not inserted
  bool insert(const T& elem) {
    auto b = internal_emplace(elem);
    if (b)
      this->publish(gset{elem});
    return b;
  }

  /// Add a set of elements into gset
  /// @param elems to insert
  /// @returns a set of operations done to the gset
  void subset_insert(const std::set<T>& elems) {
    std::set<T> insertions;
    for (auto& elem : elems)
      if (internal_emplace(elem))
        insertions.emplace(elem);
    this->publish(std::set<T>{std::move(insertions)});
  }

  /// Checks if `elem` is in the set
  /// @param elem to check
  bool element_of(const T& elem) const {
    return set_.find(elem) != set_.end();
  }

  /// Checks if `other` includes `other`.
  /// @param other set of elements
  bool is_subset_of(const std::set<T>& other) const {
    return std::includes(other.begin(), other.end(), set_.begin(), other.end());
  }

  /// Checks if `other` includes `other`.
  /// @param other set of elements
  bool is_subset_of(const gset& other) const {
    return is_subset_of(other.set_);
  }

  /// Checks if `this` includes `other`.
  /// @param other set of elements
  bool is_superset_of(const std::set<T>& other) const {
    return std::includes(set_.begin(), set_.end(), other.begin(), other.end());
  }

  /// Checks if `this` includes `other`.
  /// @param other set of elements
  bool is_superset_of(const gset& other) const {
    return is_superset_of(other.set_);
  }

  /// Checks if `other` and `this` are equal.
  bool equal(const std::set<T>& other) const {
    return set_ == other;
  }

  /// @returns the number of elements in the set
  size_t size() const { return set_.size(); }

  /// @private
  template <class Processor>
  friend void serialize(Processor& proc, gset<T>& x) {
    proc & x.set_;
  }

  intptr_t compare(const gset<T>& other) const noexcept {
    if (set_ == other.set_)     return 0;
    else if (set_ < other.set_) return -1;
    else                        return 1;
  }

  inline size_t empty() const { return set_.empty(); }

  inline typename std::set<T>::const_iterator cbegin() const {
    return set_.cbegin();
  }

  inline typename std::set<T>::const_iterator cend() const {
    return set_.cend();
  }

private:
  /// @private
  inline bool internal_emplace(const T& elem) {
    return std::get<1>(set_.emplace(elem));
  }
  std::set<T> set_; /// Set of elements
};

} // namespace types
} // namespace crdt
} // namespace caf

#endif // CAF_CRDT_TYPES_GSET_HPP
