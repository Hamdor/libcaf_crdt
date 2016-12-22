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

#define CAF_SUITE uri
#include "caf/test/unit_test.hpp"

#include "caf/all.hpp"

#include "caf/replication/uri.hpp"

using namespace caf;
using namespace caf::replication;

CAF_TEST(uri_parse) {
  std::string valid[] = {"gset<int>://videos/*",
                         "gset<string>://videos/bleh",
                         "gset<string>://videos",
                         "gset<int>://rand",
                         "gcounter<size_t>://views/videos/filmchen/*",
                         "lww_reg<int>://"
  };
  std::string invalid[] = {"",
                           "*",
                           "://",
                           "gset<int>://rand/*/"
  };
  for (auto& what : valid) CAF_CHECK(uri{what}.valid());
  for (auto& what : invalid) CAF_CHECK(!uri{what}.valid());
}

CAF_TEST(uri_rrti) {
  struct a {};
  auto cfg = actor_system_config{};
  cfg.add_message_type<a>("a");
  actor_system system{cfg};
  CAF_CHECK(uri{"a://"}.match_rtti<a>(system));
}
