#pragma once

#include "netsim/types.hpp"

#include <cstdint>
#include <deque>
#include <functional>
#include <optional>
#include <string>

namespace netsim {

enum class TcpState {
  Closed,
  Listen,
  SynSent,
  SynReceived,
  Established,
  FinWait1,
  FinWait2,
  CloseWait,
  LastAck,
  TimeWait,
};

struct TcpSegment {
  uint16_t src_port = 0;
  uint16_t dst_port = 0;
  uint32_t seq = 0;
  uint32_t ack = 0;
  uint8_t data_offset = 5;  // 20-byte header
  uint8_t flags = 0;        // bit0 FIN,1 SYN,2 RST,3 PSH,4 ACK
  uint16_t window = 65535;
  uint16_t checksum = 0;
  uint16_t urgent = 0;
  Buffer payload;

  static constexpr uint8_t FIN = 1 << 0;
  static constexpr uint8_t SYN = 1 << 1;
  static constexpr uint8_t RST = 1 << 2;
  static constexpr uint8_t PSH = 1 << 3;
  static constexpr uint8_t ACK = 1 << 4;

  Buffer serialize(Ipv4Address src, Ipv4Address dst) const;
  static bool parse(const Buffer& buf, Ipv4Address src, Ipv4Address dst, TcpSegment& out,
                    bool verify_checksum = true);
};

/** Simplified Reno-like congestion control. */
struct CongestionControl {
  uint32_t cwnd = 1;       // in MSS units
  uint32_t ssthresh = 64;
  uint32_t mss = 536;
  int dup_acks = 0;

  void on_ack_new_data();
  void on_dup_ack();
  void on_timeout();
  uint32_t send_window_bytes() const { return cwnd * mss; }
};

struct TcpPcb {
  TcpState state = TcpState::Closed;
  Ipv4Address local_ip;
  Ipv4Address remote_ip;
  uint16_t local_port = 0;
  uint16_t remote_port = 0;
  uint32_t snd_una = 0;
  uint32_t snd_nxt = 0;
  uint32_t rcv_nxt = 0;
  uint32_t iss = 1000;
  uint32_t irs = 0;
  CongestionControl cc;
  std::deque<Byte> app_rx;
  std::deque<Byte> app_tx;
  bool active_open = false;

  using EmitFn = std::function<void(const TcpSegment&)>;
  EmitFn emit;

  void listen(uint16_t port);
  void connect(Ipv4Address remote, uint16_t rport);
  void close();
  void send_app(const Buffer& data);
  Buffer recv_app(std::size_t max_n = 65535);
  void on_segment(const TcpSegment& seg);
  bool established() const { return state == TcpState::Established; }

 private:
  void emit_seg(uint8_t flags, const Buffer& payload = {});
};

}  // namespace netsim
