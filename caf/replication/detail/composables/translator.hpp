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

#ifndef CAF_REPLICATION_DETAIL_COMPOSABLES_TRANSLATOR_HPP
#define CAF_REPLICATION_DETAIL_COMPOSABLES_TRANSLATOR_HPP

#include "caf/composed_behavior.hpp"
#include "caf/composable_behavior.hpp"
#include "caf/composable_behavior_based_actor.hpp"

#include "caf/replication/replicator.hpp"

#include "caf/replication/interfaces.hpp"

#include "caf/replication/detail/composables/shared_state.hpp"

namespace caf {
namespace replication {
namespace detail {

///
template <class T>
class translator : public composable_behavior<translator_t<T>>,
                   public virtual cvrdt_buffer<T>,
                   public virtual base_state<T> {
  using delta_t = typename T::internal_t;
  using transaction_t = param_t<typename T::transaction_t>;
public:

  /// Recieve a delta-CRDT from an other CAF node
  result<void> operator()(param_t<message> msg) override {
    delta_t delta;
    msg.get_mutable().apply(
      [&](const delta_t& unpacked) {
        delta = std::move(unpacked);
      }
      // TODO: Add other handler and log errors (should not happen)
    );
    auto tmp_delta = this->merge(this->topic(), delta);
    if (tmp_delta.empty())
      return unit;
    auto t = tmp_delta.get_cmrdt_transactions(this->topic(),
                                              actor_cast<actor>(this->self));
    // Send to subscribers
    this->publish_all(this, notify_atom::value, t);
    // Send to tree childs
    this->tree_publish_all(this, publish_atom::value, std::move(t));
    return unit;
  }

  result<void> operator()(publish_atom, transaction_t transaction) override {
    const auto& trans = transaction.get();
    auto& sender = this->self->current_sender();
    // Send to tree childs
    this->tree_publish(this, sender, publish_atom::value, trans);
    // Send to other subscribers
    this->publish(this, sender, notify_atom::value, trans);
    // Apply to local CvRDT
    typename cvrdt_buffer<T>::cvrdt_type delta;
    this->apply(trans, delta);
    if (delta.empty())
      return unit;
    // Unite delta into update buffer
    this->unite(delta);
    return unit;
  }

  /// Send update buffer to replicator
  result<void> operator()(tick_atom) override {
    auto repl = this->self->system().replicator().actor_handle();
    // TODO: Send full state or delta?
    this->flush_cvrdt_buffer_to(this, repl, this->topic(), false);
    // TODO: Make configurable
    this->self->delayed_send(this->self, std::chrono::seconds(1), tick_atom::value);
    return unit;
  }
};


} // namespace detail
} // namespace replication
} // namespace caf

#endif // CAF_REPLICATION_DETAIL_COMPOSABLES_TRANSLATOR_HPP
