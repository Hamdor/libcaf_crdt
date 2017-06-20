/******************************************************************************
 *                       ____    _    _____                                   *
 *                      / ___|  / \  |  ___|    C++                           *
 *                     | |     / _ \ | |_       Actor                         *
 *                     | |___ / ___ \|  _|      Framework                     *
 *                      \____/_/   \_|_|                                      *
 *                                                                            *
 * Copyright (C) 2011 - 2017                                                  *
 * Dominik Charousset <dominik.charousset (at) haw-hamburg.de>                *
 * Marian Triebe <marian.triebe (at) haw-hamburg.de>
 *                                                                            *
 * Distributed under the terms and conditions of the BSD 3-Clause License or  *
 * (at your option) under the terms and conditions of the Boost Software      *
 * License 1.0. See accompanying files LICENSE and LICENSE_ALTERNATIVE.       *
 *                                                                            *
 * If you did not receive a copy of the license files, see                    *
 * http://opensource.org/licenses/BSD-3-Clause and                            *
 * http://www.boost.org/LICENSE_1_0.txt.                                      *
 ******************************************************************************/

#ifndef CAF_CRDT_CRDT_CONFIG_HPP
#define CAF_CRDT_CRDT_CONFIG_HPP

#include "caf/actor_system_config.hpp"

#include "caf/io/middleman.hpp"

#include "caf/crdt/detail/replica.hpp"

namespace caf {
namespace crdt {

/// Extended `actor_system_config` to support the CRDT module.
class crdt_config : public actor_system_config {
public:
  crdt_config() : actor_system_config() {
    load<io::middleman>();
    load<crdt::replicator>();
  }

  /// Adds crdt `Type` to the module
  /// @param type name of crdt
  template <class Type>
  actor_system_config& add_crdt(const std::string& name) {
    add_message_type<Type>(name);
    add_actor_type<crdt::detail::replica<Type>,
                   const uri&, const size_t&>(name);
    return *this;
  }

  /// Set the buffer flush interval (Default: 2 Seconds)
  /// @param interval in milliseconds or higher resolution (std::chrono)
  template <class Interval>
  actor_system_config& set_flush_interval(Interval interval) {
    using std::chrono::milliseconds;
    using std::chrono::duration_cast;
    crdt_flush_buffer_interval_ms = duration_cast<milliseconds>(interval).count();
    return *this;
  }

  /// Set the notify interval (Default: 500ms)
  /// @param interval in milliseconds or higher resolution (std::chrono)
  template <class Interval>
  actor_system_config& set_notify_interval(Interval interval) {
    using std::chrono::milliseconds;
    using std::chrono::duration_cast;
    crdt_notify_interval_ms = duration_cast<milliseconds>(interval).count();
    return *this;
  }

  /// Set the state interval. The complete state will be shipped
  /// in this interval. (Default: 2 Minutes)
  /// @param interval in milliseconds or higher resolution (std::chrono)
  template <class Interval>
  actor_system_config& set_state_interval(Interval interval) {
    using std::chrono::milliseconds;
    using std::chrono::duration_cast;
    crdt_state_interval_ms = duration_cast<milliseconds>(interval).count();
    return *this;
  }

  /// Set the interval id refresh interval. This node will ask other nodes
  /// for changed replic ids in the specified interval. (Default: 1 Secound)
  /// @param interval in milliseconds or higher resolution (std::chrono)
  template <class Interval>
  actor_system_config& set_refresh_ids_interval(Interval interval) {
    using std::chrono::milliseconds;
    using std::chrono::duration_cast;
    crdt_ids_interval_ms = duration_cast<milliseconds>(interval).count();
    return *this;
  }
};

} // namespace crdt
} // namespace caf

#endif // CAF_CRDT_CRDT_CONFIG_HPP
