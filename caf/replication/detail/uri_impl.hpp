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

#ifndef CAF_REPLICATION_DETAIL_URI_IMPL_HPP
#define CAF_REPLICATION_DETAIL_URI_IMPL_HPP

#include "caf/ref_counted.hpp"
#include "caf/make_counted.hpp"

namespace caf {
namespace replication {
namespace detail {

namespace {

constexpr char wildcard = '*';
constexpr char path_delim = '/';

} // namespace <anonymous>

class uri_impl : public ref_counted {

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
  static intrusive_ptr<uri_impl> from(const std::string& what) {
    auto result = make_counted<uri_impl>();
    if (!result->parse(what)) {
      result->path_.clear();
      result->scheme_.clear();
    }
    return result;
  }

  static intrusive_ptr<uri_impl> from(const char* what) {
    return from(std::string{what});
  }

  inline const std::string& scheme() const { return scheme_; }

  inline const std::string& path() const { return path_; }

  inline std::string to_string() const { return scheme_ + ":/" + path_; }

  inline bool valid() const { return !path_.empty() && !scheme_.empty(); }

private:
  std::string path_;
  std::string scheme_;
};

} // namespace detail
} // namespace replication
} // namespace caf

#endif // CAF_REPLICATION_DETAIL_URI_IMPL_HPP
