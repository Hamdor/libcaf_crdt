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

#ifndef CAF_REPLICATION_DETAIL_COMPOSABLES_SHARED_STATE_HPP
#define CAF_REPLICATION_DETAIL_COMPOSABLES_SHARED_STATE_HPP

#include "caf/replication/interfaces.hpp"

namespace caf {
namespace replication {
namespace detail {

// TODO: Checken ob alles protected ist
// TODO: id infos bauen, soll info haben über verschiedene sachen
// TODO: ...

template <class T>
struct base_state {
  using node_type = publishable<T>;
  using subs_type = notifyable<T>;
  using transaction_t = typename T::transaction_t;
protected:

  base_state() : parent_{unsafe_actor_handle_init}, childs_{} {
    // nop
  }

  ///
  void add_child(const node_type& child) {
    childs_.emplace(child);
  }

  ///
  void remove_child(const node_type& child) {
    childs_.erase(child);
  }

  ///
  void set_parent(const node_type& parent) {
    parent_ = parent;
  }

  ///
  template <class Self, class Exclude, class... Ts>
  void tree_publish(const Self& sender, const Exclude& exclude, Ts... ts) const {
    for (auto& child : childs_)
      if (child != exclude)
        sender->self->send(child, ts...);
  }

  ///
  template <class Self, class... Ts>
  void tree_publish_all(const Self& sender, Ts... ts) const {
    for (auto& child : childs_)
      sender->self->send(child, ts...);
  }

  ///
  inline const node_type& parent() const { return parent_; }

  ///
  template <class Self, class Exclude, class... Ts>
  void publish(const Self& sender, const Exclude& exclude, Ts... ts) const {
    for (auto& subscriber : subscribers_)
      if (subscriber != exclude)
        sender->self->send(subscriber, ts...);
  }

  ///
  template <class Self, class... Ts>
  void publish_all(const Self& sender, Ts... ts) const {
    for (auto& subscriber : subscribers_)
      sender->self->send(subscriber, ts...);
  }

  ///
  bool subscribe(const subs_type& subscriber) {
    return std::get<1>(subscribers_.emplace(subscriber));
  }

  ///
  void unsubscribe(const subs_type& subscriber) {
    subscribers_.erase(subscriber);
  }

  ///
  inline const std::string& topic() const { return topic_; }

  ///
  inline void set_topic(std::string topic) { topic_ = std::move(topic); }

private:
  node_type parent_;
  std::string topic_;
  std::set<node_type> childs_;
  std::set<subs_type> subscribers_;
};

///
template <class T>
class cvrdt_buffer {
protected:
  /// CvRDT State
  using cvrdt_type = typename T::internal_t;
  /// Transaction type for CmRDT
  using transaction_t = typename T::transaction_t;

  /// Merge CvRDT state with history and return delta
  /// @param history the history to apply
  ///        instance.
  void apply(const transaction_t& history, cvrdt_type& out_delta) {
    out_delta = cvrdt_state_.apply(history);
  }

  /// Merge function for two CvRDTs
  cvrdt_type merge(const std::string& topic, const cvrdt_type& other) {
    return cvrdt_state_.merge(topic, other);
  }

  ///
  void unite(const cvrdt_type& delta) {
    cvrdt_delta_.merge("", delta);
  }

  ///
  template <class From, class To>
  void flush_cvrdt_buffer_to(const From& sender, const To& to,
                             const std::string& topic, bool flush_all = false) {
    auto data = flush_all ? cvrdt_state_ : cvrdt_delta_;
    sender->self->send(to, topic, make_message(cvrdt_state_));
    cvrdt_delta_.clear();
  }

private:
  cvrdt_type cvrdt_state_;
  cvrdt_type cvrdt_delta_;
};

///
template <class T>
class cmrdt_buffer {
protected:
  /// CmRDT State, this state is propagated to local subscribers
  using cmrdt_type = T;
  /// Transaction type for CmRDT
  using transaction_t = typename cmrdt_type::transaction_t;

  /// Apply a transaction to CmRDT buffer
  void apply(const transaction_t& history) {
    cmrdt_state_.apply(history);
  }

  ///
  inline const cmrdt_type& get_cmrdt() const { return cmrdt_state_; }

  ///
  void put_buffer(const transaction_t& to) {
    cmrdt_buffer_.emplace_back(to);
  }

  ///
  template <class From, class To, class... Ts>
  void flush_cmrdt_buffer_to(const From& sender, const To& to, Ts... ts) {
    // TODO: Collapse cmrdt_buffer_
    for (auto& t : cmrdt_buffer_)
      sender->self->send(to, ts..., t);
    cmrdt_buffer_.clear();
  }

private:
  cmrdt_type cmrdt_state_;
  std::vector<transaction_t> cmrdt_buffer_;
};

} // namespace detail
} // namespace replication
} // namespace caf

#endif // CAF_REPLICATION_DETAIL_COMPOSABLES_SHARED_STATE_HPP
