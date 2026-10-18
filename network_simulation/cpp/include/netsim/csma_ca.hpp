#pragma once

#include "netsim/types.hpp"

#include <algorithm>
#include <cstdint>
#include <random>

namespace netsim {

/**
 * Teaching CSMA/CA model (IEEE 802.11 Wireless LAN):
 * - DIFS sensing before transmission
 * - Contention Window (CW) with binary exponential backoff: [0, CW]
 * - Freezing backoff timer when medium is busy, resuming after idle for DIFS
 * - Explicit link-layer ACK (receiver ACKs data frame)
 * - Optional RTS / CTS handshake for virtual carrier sensing (NAV)
 */
class CsmaCaMac {
 public:
  struct Config {
    int sifs_slots = 1;
    int difs_slots = 3;  // DIFS = SIFS + 2 * SlotTime
    int cw_min = 15;     // 802.11 standard default CWmin
    int cw_max = 1023;   // 802.11 standard default CWmax
    int max_retries = 7; // Short retry limit
    unsigned seed = 42;
  };

  struct FrameResult {
    bool success = false;
    int attempts = 0;
    int collisions = 0;
    int total_slots_waited = 0;
    bool rts_cts_used = false;
    bool ack_received = false;
  };

  CsmaCaMac();
  explicit CsmaCaMac(Config cfg);

  bool carrier_idle() const { return busy_until_slot_ <= current_slot_; }
  void set_busy_until(int slot) { busy_until_slot_ = std::max(busy_until_slot_, slot); }
  int current_slot() const { return current_slot_; }
  void advance(int slots = 1);

  int current_cw() const { return current_cw_; }

  /**
   * Attempt to send a packet over CSMA/CA.
   * If use_rts_cts is true, simulates RTS/CTS 4-way exchange: RTS -> SIFS -> CTS -> SIFS -> DATA -> SIFS -> ACK.
   * If collide_first_attempt is true, simulates collision on the initial attempt.
   */
  FrameResult transmit(int data_slots, bool use_rts_cts = false, bool collide_first_attempt = false);

  int pick_backoff_slots();

 private:
  Config cfg_;
  std::mt19937 rng_;
  int current_slot_ = 0;
  int busy_until_slot_ = 0;
  int current_cw_ = 15;
};

}  // namespace netsim

