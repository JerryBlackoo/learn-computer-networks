#pragma once

#include "netsim/types.hpp"

#include <cstdint>
#include <deque>
#include <string>
#include <vector>

namespace netsim {

/**
 * Teaching IEEE 802.5 Token Ring model:
 * - Round-robin controlled access (no collisions)
 * - Free token circulates around the logical ring
 * - A station with data captures free token, marks it busy, transmits data frame
 * - Destination station copies data, sets Address-Recognized (A) and Frame-Copied (C) bits
 * - Frame circles back to source station; source station strips frame and releases free token
 */
class TokenRing {
 public:
  struct FrameStatus {
    bool address_recognized = false; // 'A' bit
    bool frame_copied = false;        // 'C' bit
  };

  struct RingFrame {
    bool is_token = true;
    bool is_busy = false; // false = free token, true = data frame
    int src_node = -1;
    int dst_node = -1;
    Buffer payload;
    FrameStatus status;
  };

  explicit TokenRing(int num_stations);

  int station_count() const { return num_stations_; }
  int current_token_holder() const { return token_pos_; }

  /** Enqueue a data frame to be transmitted by a specific station */
  void enqueue(int src_node, int dst_node, Buffer data);

  /** Get received frames for a destination station */
  const std::vector<Buffer>& get_received(int node) const;

  /** Step the ring: moves the token or in-flight frame one hop downstream */
  bool step();

  /** Run the ring until all enqueued frames are transmitted and delivered */
  int run_until_idle(int max_steps = 100);

  int total_frames_delivered() const { return frames_delivered_; }
  int total_tokens_circulated() const { return tokens_passed_; }

 private:
  int num_stations_ = 4;
  int token_pos_ = 0; // Current station with the token or frame
  RingFrame in_flight_;
  bool has_in_flight_data_ = false;

  std::vector<std::deque<RingFrame>> station_queues_;
  std::vector<std::vector<Buffer>> station_received_;

  int frames_delivered_ = 0;
  int tokens_passed_ = 0;
};

}  // namespace netsim

