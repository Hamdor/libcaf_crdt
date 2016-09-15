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

#include "caf/replication/uri.hpp"

#include <iostream>

using namespace caf::replication;

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

  return 0;
}
