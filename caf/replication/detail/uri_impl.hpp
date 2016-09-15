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
    parse_slash_one,
    parse_slash_two,
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
          state_ = states::parse_slash_one;
        else
          scheme_ += c;
        return true;
      case states::parse_slash_one:
        return check(states::parse_slash_two, path_delim);
      case states::parse_slash_two:
        return check(states::parse_topic, path_delim);
      case states::parse_topic:
        path_buffer_ += c;
        if (c == wildcard)
          state_ = states::parse_end;
        if (c == path_delim) {
          if (path_buffer_.empty())
            return false;
          path_.emplace_back(path_buffer_);
          path_buffer_.clear();
        }
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
      uri_ += c;
      if (!consume(c))
        return false;
    }
    if (!path_buffer_.empty())
      path_.emplace_back(path_buffer_);
    if (scheme_.empty() || path_.empty())
      return false;
    return true;
  }

public:
  static intrusive_ptr<uri_impl> from(const std::string& what) {
    auto result = make_counted<uri_impl>();
    result->valid_ = result->parse(what);
    return result;
  }

  static intrusive_ptr<uri_impl> from(const char* what) {
    return from(std::string{what});
  }

  inline const std::string& scheme() const { return scheme_; }

  inline std::string path() const {
    std::string path;
    for (auto& step : path_)
      path += step;
    return path;
  }

  inline size_t path_deep() const { return path_.size(); }

  inline const std::string& path_at(size_t deep) const {
    return deep < path_deep() ? path_[deep] : path_buffer_;
  }

  inline const std::string& to_string() const { return uri_; }

  inline bool valid() const { return valid_; }

private:
  std::string path_buffer_;
  std::vector<std::string> path_;

  std::string scheme_;
  std::string uri_;

  bool valid_;
};

} // namespace detail
} // namespace replication
} // namespace caf

#endif // CAF_REPLICATION_DETAIL_URI_IMPL_HPP
