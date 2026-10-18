#include "netsim/phy.hpp"

namespace netsim {

PhyLink::PhyLink(PhyParams p) : params_(p), rng_(p.seed) {}

void PhyLink::send(const Buffer& frame) {
  std::uniform_real_distribution<double> u(0.0, 1.0);
  if (u(rng_) < params_.loss_prob) return;

  Buffer copy = frame;
  if (params_.bit_error_prob > 0 && !copy.empty()) {
    for (auto& b : copy) {
      for (int bit = 0; bit < 8; ++bit) {
        if (u(rng_) < params_.bit_error_prob) {
          b ^= static_cast<Byte>(1u << bit);
        }
      }
    }
  }
  rx_.push_back(std::move(copy));
}

bool PhyLink::receive(Buffer& out) {
  if (rx_.empty()) return false;
  out = std::move(rx_.front());
  rx_.pop_front();
  return true;
}

void PhyLink::clear() { rx_.clear(); }

}  // namespace netsim
