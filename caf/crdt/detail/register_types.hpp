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

#ifndef CAF_CRDT_DETAIL_REGISTER_TYPES_HPP
#define CAF_CRDT_DETAIL_REGISTER_TYPES_HPP

#include "caf/actor_system_config.hpp"

#include "caf/crdt/types/all.hpp"

#include "caf/crdt/detail/replica.hpp"

#include "caf/crdt/detail/distribution_layer.hpp"

namespace caf {
namespace crdt {
namespace detail {

struct register_types {

  register_types(actor_system_config& cfg) : cfg_(cfg) {
    // nop
  }

  void operator()() noexcept {
    cfg_.add_message_type<uri>("uri");
    cfg_.add_message_type<distribution_layer::tuple_type>("dist_tuple");
    cfg_.add_message_type<distribution_layer::map_type>("dist_map");
    // ...

    // ...
    cfg_.add_message_type<types::base_datatype>("base_datatype");
    cfg_.add_message_type<uri>("uri");
    cfg_.add_message_type<std::unordered_set<uri>>("unordered_set<uri>");
    cfg_.add_message_type<types::gmap<node_id, std::pair<size_t, std::unordered_set<uri>>>>("gmap_distlayer");
    // TODO: Base klasse benötigt?
    // Datentypen wie z.B. gset<node_id> registrieren
    //                     gset<actor_addr>, gset<actor>,...
  }

private:
  actor_system_config& cfg_;
};

} // namespace detail
} // namespace crdt
} // namespace caf

#endif // CAF_CRDT_DETAIL_REGISTER_TYPES_HPP
