

#ifndef CAF_CRDT_LAMPORT_CLOCK_HPP
#define CAF_CRDT_LAMPORT_CLOCK_HPP

#include "caf/detail/comparable.hpp"

namespace caf {
namespace crdt {

/// Implementation of a lamport clock
struct lamport_clock : caf::detail::comparable<lamport_clock>,
                       caf::detail::comparable<lamport_clock, uint64_t> {
  lamport_clock() : time_{0} {
    // nop
  }

  lamport_clock(uint64_t value) : time_{value} {
    // nop
  }

  /// Merge two lamport clocks
  lamport_clock merge(const lamport_clock& other) {
    time_ = std::max(time_, other.time_);
    return {time_};
  }

  /// @returns the current time
  uint64_t time() const { return time_; }

  /// @returns post-increment internal time and returns its value
  uint64_t increment() { return ++time_; }

  /// @cond PRIVATE

  intptr_t compare(uint64_t lhs, uint64_t rhs) const noexcept {
    return static_cast<intptr_t>(lhs - rhs);
  }

  intptr_t compare(const lamport_clock& other) const noexcept {
    return compare(time_, other.time_);
  }

  intptr_t compare(uint64_t other) const noexcept {
    return compare(time_, other);
  }

  /// @private
  template <class Processor>
  friend void serialize(Processor& proc, lamport_clock& x) {
    proc & x.time_;
  }

private:
  uint64_t time_;
};

} // namespace crdt
} // namespace caf

#endif // CAF_CRDT_LAMPORT_CLOCK_HPP
