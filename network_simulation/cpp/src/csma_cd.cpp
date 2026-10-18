#include "netsim/csma_cd.hpp"

#include <algorithm>

namespace netsim {

CsmaCdMac::CsmaCdMac() : cfg_{}, rng_(cfg_.seed) {}

CsmaCdMac::CsmaCdMac(Config cfg) : cfg_(cfg), rng_(cfg.seed) {}

void CsmaCdMac::advance(int slots) { current_slot_ += slots; }

int CsmaCdMac::backoff_slots(int collision_count) {
  int k = std::max(0, collision_count);
  int m = std::min(k, 10);
  int maxv = (1 << m) - 1;
  std::uniform_int_distribution<int> dist(0, std::max(0, maxv));
  return dist(rng_);
}

CsmaCdMac::AttemptResult CsmaCdMac::try_transmit(int tx_slots, bool collide_this_attempt) {
  AttemptResult r;
  while (r.attempts < cfg_.max_attempts) {
    ++r.attempts;
    // Carrier sense: wait until idle
    if (!carrier_idle()) {
      int wait = busy_until_slot_ - current_slot_;
      advance(wait);
      r.total_slots_waited += wait;
    }
    // Start transmission
    if (collide_this_attempt && r.collisions == 0) {
      // First attempt collides: jam + backoff
      ++r.collisions;
      advance(cfg_.jam_slots);
      r.total_slots_waited += cfg_.jam_slots;
      int bo = backoff_slots(r.collisions);
      advance(bo);
      r.total_slots_waited += bo;
      collide_this_attempt = false;  // subsequent attempts succeed in this teaching model
      continue;
    }
    // Success: occupy medium
    set_busy_until(current_slot_ + tx_slots);
    advance(tx_slots);
    r.success = true;
    return r;
  }
  r.success = false;
  return r;
}

}  // namespace netsim
