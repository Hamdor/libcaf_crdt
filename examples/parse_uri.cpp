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

#include "caf/all.hpp"

#include "caf/replication/all.hpp"

#include "caf/replication/uri.hpp"
#include "caf/replication/topic.hpp"

#include <set>
#include <iostream>

using namespace caf;
using namespace caf::replication;

struct some_crap {};

int main() {
  std::string tests[] = {"gset<int>://videos/*",
                         "gset<string>://videos/bleh",
                         "gset<string>://videos",
                         "gset<int>://rand",
                         "gcounter<size_t>://views/videos/filmchen/*"
  };
  for (auto& what : tests) {
    uri u{what};
    if (u.valid())
      std::cout << "scheme/type: " << u.scheme()    << std::endl
                << "path/topic:  " << u.path()      << std::endl
                << "uri:         " << u.to_string() << std::endl;
    else
      std::cout << "invalid uri (" << what << ")" << std::endl;
    std::cout << "-----------------------------------------------------"
              << std::endl;
  }
  // ----
  auto cfg = actor_system_config{};
  cfg.add_message_type<crdt::gset<int>>("gset<int>");
  actor_system system{cfg};
  // ----
  std::cout << topic<crdt::gset<int>>(system, "/rand").get_uri().to_string()
            << std::endl;
  std::cout << topic<some_crap>(system, "/rand").get_uri().to_string()
            << std::endl;
  auto u = uri{"gset<int>://rand"};
  std::cout << std::boolalpha << "assumed (true, false): "
            << u.match_rtti<crdt::gset<int>>(system) << ", "
            << u.match_rtti<crdt::gset<float>>(system) << std::endl;
  return 0;
}
