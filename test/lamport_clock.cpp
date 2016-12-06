#define CAF_SUITE lamport_clock
#include "caf/test/unit_test.hpp"

#include "caf/replication/lamport_clock.hpp"

using namespace caf::replication;

CAF_TEST(compare) {
  lamport_clock l1, l2;
  CAF_CHECK(l1 == l2);
  CAF_CHECK(!(l1 < l2));
  CAF_CHECK(!(l1 > l2));
  CAF_CHECK(l1 == l2.time());
  // ...
  CAF_CHECK(l1.increment() == 1);
  CAF_CHECK(l1 != l2);
  CAF_CHECK(l1 > l2);
  CAF_CHECK(l1 == l2.increment());
  l1.merge({42});
  CAF_CHECK(l1 == 42);
}
