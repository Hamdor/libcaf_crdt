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

#ifndef CAF_REPLICATION_TOPIC_HPP
#define CAF_REPLICATION_TOPIC_HPP

#include "caf/replication/uri.hpp"

namespace caf {
namespace replication {

/// A topic is defined by a scheme (type) and a path, it contains
/// a underlying uri.
template <class T>
struct topic {
  /// Underlying type of topic
  using scheme_type = T;

  /// Creates a new topic
  /// @param system the actor system which knows type T
  /// @param what valid path
  topic(const actor_system& system, const std::string& what)
      : uri_(static_cast<T*>(nullptr), system, what) {
    // nop
  }

  /// Get the underlying uri
  const uri& get_uri() const { return uri_; }

private:
  uri uri_; /// Underlying URI
};

} // namespace replication
} // namespace caf


#endif // CAF_REPLICATION_TOPIC_HPP
