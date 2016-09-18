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

#ifndef CAF_REPLICATION_DETAIL_COMPOSABLES_SUBSCRIBABLE_HPP
#define CAF_REPLICATION_DETAIL_COMPOSABLES_SUBSCRIBABLE_HPP

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
class subscribable : public composable_behavior<subscribable_t<T>>,
                     public virtual cmrdt_buffer<T>,
                     public virtual base_state<T> {

  using init_type  = result<initial_atom, T>;
  using subs_param = param_t<notifyable<T>>;
  using node_arg   = param_t<publishable<T>>;

  T init_state(const actor& owner) const {
    T t{this->get_cmrdt()};
    t.set_owner(owner);
    t.set_parent(actor_cast<actor>(this->self));
    return t;
  }

public:
  init_type operator()(subscribe_atom, subs_param subscriber) override {
    auto& subs = subscriber.get();
    // Check if subscriber is in the same node, if not return a error
    //if (this->self->node() != subs.node())
    //  return make_error();
    /*auto added = */this->subscribe(subs);
    //if (! added)
    //  return unit;
    // Initially send full state
    auto t = init_state(actor_cast<actor>(subs));
    return {initial_atom::value, std::move(t)};
  }

  result<void> operator()(unsubscribe_atom, subs_param subscriber) override {
    this->unsubscribe(subscriber.get());
    return unit;
  }

  result<void> operator()(set_parent_atom, node_arg parent) override {
    this->set_parent(parent);
    return unit;
  }

  result<void> operator()(add_child_atom, node_arg parent) override {
    this->add_child(parent);
    return unit;
  }
};


} // namespace detail
} // namespace replication
} // namespace caf

#endif // CAF_REPLICATION_DETAIL_COMPOSABLES_SUBSCRIBABLE_HPP
