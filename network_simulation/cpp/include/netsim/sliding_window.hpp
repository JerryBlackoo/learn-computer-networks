#pragma once

#include "netsim/types.hpp"

#include <cstdint>
#include <deque>
#include <functional>
#include <optional>
#include <vector>

namespace netsim {

/** Go-Back-N sliding window over discrete frames (teaching model). */
class GbnSender {
 public:
  using SendFn = std::function<void(uint32_t seq, const Buffer& data)>;

  GbnSender(uint32_t window_size, SendFn send_fn);

  /** Queue application data; may trigger sends inside window. */
  void enqueue(Buffer data);
  /** Cumulative ACK: all seq <= ack_seq confirmed. */
  void on_ack(uint32_t ack_seq);
  /** Timeout: retransmit from base. */
  void on_timeout();
  bool idle() const { return next_seq_ == base_ && queue_.empty(); }
  uint32_t base() const { return base_; }
  uint32_t next_seq() const { return next_seq_; }

 private:
  void try_send();

  uint32_t window_;
  SendFn send_fn_;
  uint32_t base_ = 0;
  uint32_t next_seq_ = 0;
  std::deque<Buffer> queue_;
  std::vector<Buffer> unacked_;
};

class GbnReceiver {
 public:
  using DeliverFn = std::function<void(const Buffer& data)>;
  using AckFn = std::function<void(uint32_t ack_seq)>;

  GbnReceiver(DeliverFn deliver, AckFn ack);

  void on_frame(uint32_t seq, const Buffer& data);

 private:
  uint32_t expected_ = 0;
  DeliverFn deliver_;
  AckFn ack_;
};

}  // namespace netsim
