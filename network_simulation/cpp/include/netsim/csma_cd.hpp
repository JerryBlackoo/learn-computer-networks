#pragma once

#include "netsim/types.hpp"

#include <algorithm>
#include <cstdint>
#include <random>

namespace netsim {

/**
 * Teaching CSMA/CD model (Ethernet-style):
 * - Carrier sense before transmit
 * - Collision detection during transmission slot
 * - Binary exponential backoff after collision
 *
 * Times are discrete "slot times" (not wall-clock).
 */
class CsmaCdMac {
 public:
  struct Config {
    int max_attempts = 16;
    int jam_slots = 1;
    unsigned seed = 7;
  };

  CsmaCdMac();
  explicit CsmaCdMac(Config cfg);

  /** True if medium currently idle (no ongoing TX from any station we track). */
  bool carrier_idle() const { return busy_until_slot_ <= current_slot_; }

  void set_busy_until(int slot) { busy_until_slot_ = std::max(busy_until_slot_, slot); }
  int current_slot() const { return current_slot_; }
  void advance(int slots = 1);

  /**
   * Attempt to send a frame occupying `tx_slots` slot times.
   * Returns nullopt on success (scheduled), or collision count if aborted after max attempts.
   * On each collision, applies binary exponential backoff and retries.
   */
  struct AttemptResult {
    bool success = false;
    int collisions = 0;
    int total_slots_waited = 0;
    int attempts = 0;
  };
  AttemptResult try_transmit(int tx_slots, bool collide_this_attempt);

  /** Classic BEB: after k collisions, wait U{0..2^m-1} slots, m=min(k,10). */
  int backoff_slots(int collision_count);

 private:
  Config cfg_;
  std::mt19937 rng_;
  int current_slot_ = 0;
  int busy_until_slot_ = 0;
};

/** Minimum Ethernet frame size constraint helper (bytes, teaching). */
inline bool ethernet_min_frame_ok(std::size_t frame_bytes_without_preamble) {
  return frame_bytes_without_preamble >= 64;
}

}  // namespace netsim
