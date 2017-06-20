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

#ifndef CAF_CRDT_URI_HPP
#define CAF_CRDT_URI_HPP

#include "caf/actor_system.hpp"
#include "caf/intrusive_ptr.hpp"

#include "caf/detail/comparable.hpp"

namespace caf {
namespace crdt {

/// Uri implementation, that supports a scheme and a path.
class uri : caf::detail::comparable<uri> {
  static constexpr char wildcard = '*';
  static constexpr char path_delim = '/';
public:
  uri() = default;

  /// Creates a uri from a string
  /// @param what string that contains a uri
  uri(const std::string& what) {
    parse(what);
  }

  /// Creates a uri from a C-String
  /// @param what C-String that contains a uri
  uri(const char* what) {
    parse(what);
  }

  /// @returns the scheme of the uri
  inline const std::string& scheme() const { return scheme_; }

  /// @returns the path of the uri
  inline const std::string& path() const { return path_; }

  /// @returns the string of the uri
  inline std::string to_string() const { return scheme_ + ":/" + path_; }

  /// @returns `true` if a valid uri is loaded otherwise `false`.
  inline bool valid() const { return !path_.empty() && !scheme_.empty(); }

  /// @private
  intptr_t compare(const uri& other) const noexcept {
    return !(path_ == other.path_ && scheme_ == other.scheme_);
  }

private:
  enum class states {
    parse_scheme,
    parse_slash,
    parse_id,
    parse_end
  } state_;

  bool consume(char c) {
    auto check = [&](const states& next, char assumed) {
      if (c != assumed)
        return false;
      state_ = next;
      return true;
    };
    switch(state_) {
      case states::parse_scheme:
        if (c == ':')
          state_ = states::parse_slash;
        else
          scheme_ += c;
        return true;
      case states::parse_slash:
        return check(states::parse_id, path_delim);
      case states::parse_id:
        path_ += c;
        if (c == wildcard)
          state_ = states::parse_end;
        return true;
      case states::parse_end:
        return false;
    }
    return false;
  }

  bool parse(const std::string& what) {
    state_ = states::parse_scheme;
    for (size_t i = 0; i < what.size(); ++i) {
      char c = std::tolower(what[i]);
      if (!consume(c))
        return false;
    }
    return !scheme_.empty() && !path_.empty();
  }

  /// @private
  template <class Processor>
  friend void serialize(Processor& proc, uri& x) {
    proc & x.path_;
    proc & x.scheme_;
  }

  std::string path_;   /// Path of uri
  std::string scheme_; /// Scheme of uri
};

} // namespace crdt
} // namespace caf

namespace std {

template <>
struct hash<caf::crdt::uri> {
  inline size_t operator()(const caf::crdt::uri& u) const {
    return hash<std::string>()(u.to_string());
  }
};

} // namespace std

#endif // CAF_CRDT_URI_HPP
