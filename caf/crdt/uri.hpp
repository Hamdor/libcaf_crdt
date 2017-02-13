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

#ifndef CAF_CRDT_URI_HPP
#define CAF_CRDT_URI_HPP

#include "caf/actor_system.hpp"
#include "caf/intrusive_ptr.hpp"

#include "caf/detail/comparable.hpp"

namespace {

constexpr char wildcard = '*';
constexpr char path_delim = '/';

} // namespace <anonymous>

namespace caf {
namespace crdt {

/// Uri implementation, that supports a scheme and a path.
class uri : public caf::detail::comparable<uri> {
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

  /// Creates a uri from a path string and a type
  /// @param system which knows type T
  /// @param path of uri
  template <class T>
  uri(T*, const actor_system& system, const std::string& path) {
    auto nr = type_nr<T>::value;
    auto* name = (nr != 0) ? system.types().portable_name(nr, nullptr)
                           : system.types().portable_name(0, &typeid(T));
    std::string divider = ":/";
    from((name ? *name : "<unknown>") + divider + path);
  }

  /// @returns the scheme of the uri
  inline const std::string& scheme() const { return scheme_; }

  /// @returns the path of the uri
  inline const std::string& path() const { return path_; }


  inline std::string str() const { return to_string(); }

  /// @returns `true` if type is valid in our actor system
  template <class T>
  bool match_rtti(const actor_system& system) const {
    auto nr = type_nr<T>::value;
    auto* name = (nr != 0) ? system.types().portable_name(nr, nullptr)
                           : system.types().portable_name(0, &typeid(T));
    return name != nullptr && std::string{*name} == scheme_;
  }

  enum class states {
    parse_scheme,
    parse_slash,
    parse_topic,
    parse_end
  } state_;

  bool consume(char c) {
    // -----
    auto check = [&](const states& next, char assumed) {
      if (c != assumed)
        return false;
      state_ = next;
      return true;
    };
    // -----
    switch(state_) {
      case states::parse_scheme:
        if (c == ':')
          state_ = states::parse_slash;
        else
          scheme_ += c;
        return true;
      case states::parse_slash:
        return check(states::parse_topic, path_delim);
      case states::parse_topic:
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

public:

  inline std::string to_string() const { return scheme_ + ":/" + path_; }

  /// @returns `true` if a valid uri is loaded otherwise `false`.
  inline bool valid() const { return !path_.empty() && !scheme_.empty(); }

  /// @private
  template <class Processor>
  friend void serialize(Processor& proc, uri& x) {
    proc & x.path_;
    proc & x.scheme_;
  }

  intptr_t compare(const uri& other) const noexcept {
    return !(path_ == other.path_ && scheme_ == other.scheme_);
  }

private:
  std::string path_;
  std::string scheme_;
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
