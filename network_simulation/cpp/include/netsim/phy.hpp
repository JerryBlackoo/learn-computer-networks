#pragma once

#include "netsim/types.hpp"

#include <deque>
#include <random>

namespace netsim {

struct PhyParams {
  double loss_prob = 0.0;
  double bit_error_prob = 0.0;
  unsigned seed = 42;
};

/** Unreliable byte-pipe between two adjacent nodes (physical-layer abstraction). */
class PhyLink {
 public:
  explicit PhyLink(PhyParams p = {});

  void send(const Buffer& frame);
  bool receive(Buffer& out);  // pop one frame if available
  void clear();
  std::size_t pending() const { return rx_.size(); }

 private:
  PhyParams params_;
  std::mt19937 rng_;
  std::deque<Buffer> rx_;
};

}  // namespace netsim
