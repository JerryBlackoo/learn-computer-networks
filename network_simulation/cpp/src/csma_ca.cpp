#include "netsim/csma_ca.hpp"

#include <algorithm>

namespace netsim {

CsmaCaMac::CsmaCaMac() : cfg_{}, rng_(cfg_.seed), current_cw_(cfg_.cw_min) {}

CsmaCaMac::CsmaCaMac(Config cfg)
    : cfg_(cfg), rng_(cfg.seed), current_cw_(cfg.cw_min) {}

void CsmaCaMac::advance(int slots) { current_slot_ += slots; }

int CsmaCaMac::pick_backoff_slots() {
  std::uniform_int_distribution<int> dist(0, current_cw_);
  return dist(rng_);
}

CsmaCaMac::FrameResult CsmaCaMac::transmit(int data_slots, bool use_rts_cts,
                                           bool collide_first_attempt) {
  FrameResult r;
  r.rts_cts_used = use_rts_cts;

  while (r.attempts < cfg_.max_retries) {
    ++r.attempts;

    // 1. Initial sense: if busy, wait until idle
    if (!carrier_idle()) {
      int wait = busy_until_slot_ - current_slot_;
      advance(wait);
      r.total_slots_waited += wait;
    }

    // 2. Wait for DIFS idle duration
    advance(cfg_.difs_slots);
    r.total_slots_waited += cfg_.difs_slots;

    // 3. Contention Window Backoff
    int bo = pick_backoff_slots();
    advance(bo);
    r.total_slots_waited += bo;

    // 4. Simulate collision on initial attempt if requested
    if (collide_first_attempt && r.collisions == 0) {
      ++r.collisions;
      // Collision occurs -> double Contention Window (Binary Exponential Backoff)
      current_cw_ = std::min(cfg_.cw_max, 2 * (current_cw_ + 1) - 1);
      collide_first_attempt = false; // next attempt succeeds
      continue;
    }

    // 5. Successful transmission
    if (use_rts_cts) {
      // 4-Way Handshake: RTS -> SIFS -> CTS -> SIFS -> DATA -> SIFS -> ACK
      int rts_slots = 1;
      int cts_slots = 1;
      int ack_slots = 1;
      int handshake_slots = rts_slots + cfg_.sifs_slots + cts_slots +
                            cfg_.sifs_slots + data_slots + cfg_.sifs_slots +
                            ack_slots;
      set_busy_until(current_slot_ + handshake_slots);
      advance(handshake_slots);
    } else {
      // Basic 2-Way: DATA -> SIFS -> ACK
      int ack_slots = 1;
      int total_tx = data_slots + cfg_.sifs_slots + ack_slots;
      set_busy_until(current_slot_ + total_tx);
      advance(total_tx);
    }

    // Reset Contention Window to CWmin on success
    current_cw_ = cfg_.cw_min;
    r.ack_received = true;
    r.success = true;
    return r;
  }

  r.success = false;
  return r;
}

}  // namespace netsim

