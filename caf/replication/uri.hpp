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

#ifndef CAF_REPLICATION_URI_HPP
#define CAF_REPLICATION_URI_HPP

#include "caf/intrusive_ptr.hpp"

#include "caf/replication/detail/uri_impl.hpp"

namespace caf {
namespace replication {

/// Uri implementation, that supports a scheme and a path.
class uri {
  using impl = detail::uri_impl;
public:
  /// Creates a uri from a string
  /// @param what string that contains a uri
  uri(const std::string& what) : impl_(impl::from(what)) {
    // nop
  }

  /// Creates a uri from a C-String
  /// @param what C-String that contains a uri
  uri(const char* what) : impl_(impl::from(what)) {
    // nop
  }

  /// @returns `true` if a valid uri is loaded otherwise `false`.
  bool valid() const { return impl_->valid(); }

  /// @returns the scheme of the uri
  inline const std::string& scheme() const { return impl_->scheme(); }

  /// @returns the path of the uri
  inline const std::string& path() const { return impl_->path(); }

  /// @returns the complete uri as string
  inline std::string to_string() const { return impl_->to_string(); }

  /// TODO
  template <class T>
  bool match_rtti(std::type_info* rtti) const {
    return false;
  }

  /// TODO
  template <class T>
  std::type_info* get_rtti() const {
    return nullptr;
  }

private:
  intrusive_ptr<impl> impl_;
};


} // namespace replication
} // namespace caf

#endif // CAF_REPLICATION_URI_HPP
