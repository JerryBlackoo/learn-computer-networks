#include "netsim/sliding_window.hpp"

namespace netsim {

GbnSender::GbnSender(uint32_t window_size, SendFn send_fn)
    : window_(window_size), send_fn_(std::move(send_fn)) {}

void GbnSender::enqueue(Buffer data) {
  queue_.push_back(std::move(data));
  try_send();
}

void GbnSender::try_send() {
  while (next_seq_ < base_ + window_ && !queue_.empty()) {
    Buffer data = std::move(queue_.front());
    queue_.pop_front();
    if (unacked_.size() < (next_seq_ - base_ + 1)) {
      unacked_.resize(next_seq_ - base_ + 1);
    }
    // store relative to base
    std::size_t idx = next_seq_ - base_;
    if (unacked_.size() <= idx) unacked_.resize(idx + 1);
    unacked_[idx] = data;
    send_fn_(next_seq_, data);
    ++next_seq_;
  }
}

void GbnSender::on_ack(uint32_t ack_seq) {
  if (ack_seq < base_) return;
  uint32_t advance = ack_seq - base_ + 1;
  if (advance > unacked_.size()) advance = static_cast<uint32_t>(unacked_.size());
  unacked_.erase(unacked_.begin(), unacked_.begin() + advance);
  base_ = ack_seq + 1;
  try_send();
}

void GbnSender::on_timeout() {
  for (uint32_t seq = base_; seq < next_seq_; ++seq) {
    std::size_t idx = seq - base_;
    if (idx < unacked_.size()) send_fn_(seq, unacked_[idx]);
  }
}

GbnReceiver::GbnReceiver(DeliverFn deliver, AckFn ack)
    : deliver_(std::move(deliver)), ack_(std::move(ack)) {}

void GbnReceiver::on_frame(uint32_t seq, const Buffer& data) {
  if (seq == expected_) {
    deliver_(data);
    ack_(expected_);
    ++expected_;
  } else if (expected_ > 0) {
    ack_(expected_ - 1);  // re-ACK last good
  }
}

}  // namespace netsim
