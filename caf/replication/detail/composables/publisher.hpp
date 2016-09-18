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

#ifndef CAF_REPLICATION_DETAIL_COMPOSABLES_PUBLISHER_HPP
#define CAF_REPLICATION_DETAIL_COMPOSABLES_PUBLISHER_HPP

#include "caf/composed_behavior.hpp"
#include "caf/composable_behavior.hpp"
#include "caf/composable_behavior_based_actor.hpp"

#include "caf/replication/interfaces.hpp"

#include "caf/replication/detail/composables/shared_state.hpp"

namespace caf {
namespace replication {
namespace detail {

///
template <class T>
class publisher : public composable_behavior<publishable<T>>,
                  public virtual base_state<T>,
                  public virtual cmrdt_buffer<T> {
  using transaction_t = param_t<typename T::transaction_t>;

public:

  result<void> operator()(publish_atom, transaction_t transaction) override {
    const auto& t = transaction.get();
    // Apply to our state
    this->apply(t);
    // Send to tree childs
    auto& sender = this->self->current_sender();
    if (sender != this->parent())
      this->put_buffer(t);
    // Send to our tree childs
    this->tree_publish(this, sender, publish_atom::value, t);
    // Send to subscribers
    this->publish(this, sender, notify_atom::value, t);
    return unit;
  }

  /// Send update buffer to our parent
  result<void> operator()(tick_atom) override {
    this->flush_cmrdt_buffer_to(this, this->parent(), publish_atom::value);
    // TODO: Make configurable
    this->self->delayed_send(this->self, std::chrono::seconds(1),
                             tick_atom::value);
    return unit;
  }
};

} // namespace detail
} // namespace replication
} // namespace caf

#endif // CAF_REPLICATION_DETAIL_COMPOSABLES_PUBLISHER_HPP
