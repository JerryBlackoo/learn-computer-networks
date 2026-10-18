#include "netsim/tcp.hpp"

#include "netsim/checksum.hpp"

#include <algorithm>

namespace netsim {

namespace {
Buffer tcp_pseudo(Ipv4Address src, Ipv4Address dst, uint16_t tcp_len, const Byte* tcp,
                  std::size_t n) {
  Buffer p(12 + n);
  src.write_bytes(p.data());
  dst.write_bytes(p.data() + 4);
  p[8] = 0;
  p[9] = 6;
  write_u16_be(p.data() + 10, tcp_len);
  std::copy(tcp, tcp + n, p.begin() + 12);
  return p;
}
}  // namespace

Buffer TcpSegment::serialize(Ipv4Address src, Ipv4Address dst) const {
  std::size_t hdr = data_offset * 4;
  Buffer out(hdr + payload.size());
  write_u16_be(out.data(), src_port);
  write_u16_be(out.data() + 2, dst_port);
  write_u32_be(out.data() + 4, seq);
  write_u32_be(out.data() + 8, ack);
  out[12] = static_cast<Byte>((data_offset << 4) & 0xF0);
  out[13] = flags;
  write_u16_be(out.data() + 14, window);
  write_u16_be(out.data() + 16, 0);
  write_u16_be(out.data() + 18, urgent);
  std::copy(payload.begin(), payload.end(), out.begin() + hdr);
  Buffer pseudo = tcp_pseudo(src, dst, static_cast<uint16_t>(out.size()), out.data(), out.size());
  write_u16_be(out.data() + 16, internet_checksum(pseudo));
  return out;
}

bool TcpSegment::parse(const Buffer& buf, Ipv4Address src, Ipv4Address dst, TcpSegment& out,
                       bool verify_checksum) {
  if (buf.size() < 20) return false;
  out.src_port = read_u16_be(buf.data());
  out.dst_port = read_u16_be(buf.data() + 2);
  out.seq = read_u32_be(buf.data() + 4);
  out.ack = read_u32_be(buf.data() + 8);
  out.data_offset = (buf[12] >> 4) & 0x0F;
  out.flags = buf[13];
  out.window = read_u16_be(buf.data() + 14);
  out.checksum = read_u16_be(buf.data() + 16);
  out.urgent = read_u16_be(buf.data() + 18);
  std::size_t hdr = out.data_offset * 4;
  if (hdr < 20 || hdr > buf.size()) return false;
  if (verify_checksum) {
    Buffer pseudo = tcp_pseudo(src, dst, static_cast<uint16_t>(buf.size()), buf.data(), buf.size());
    if (internet_checksum(pseudo) != 0) return false;
  }
  out.payload.assign(buf.begin() + hdr, buf.end());
  return true;
}

void CongestionControl::on_ack_new_data() {
  dup_acks = 0;
  if (cwnd < ssthresh) {
    cwnd += 1;
  } else {
    static thread_local uint32_t acc = 0;
    acc += 1;
    if (acc >= cwnd) {
      cwnd += 1;
      acc = 0;
    }
  }
}

void CongestionControl::on_dup_ack() {
  ++dup_acks;
  if (dup_acks == 3) {
    ssthresh = std::max(cwnd / 2, 2u);
    cwnd = ssthresh + 3;
  } else if (dup_acks > 3) {
    cwnd += 1;
  }
}

void CongestionControl::on_timeout() {
  ssthresh = std::max(cwnd / 2, 2u);
  cwnd = 1;
  dup_acks = 0;
}

void TcpPcb::emit_seg(uint8_t flags, const Buffer& payload) {
  if (!emit) return;
  TcpSegment seg;
  seg.src_port = local_port;
  seg.dst_port = remote_port;
  seg.seq = snd_nxt;
  seg.ack = rcv_nxt;
  seg.flags = flags;
  seg.payload = payload;

  if (flags & TcpSegment::SYN) ++snd_nxt;
  if (!payload.empty()) snd_nxt += static_cast<uint32_t>(payload.size());
  if (flags & TcpSegment::FIN) ++snd_nxt;

  emit(seg);
}

void TcpPcb::listen(uint16_t port) {
  local_port = port;
  state = TcpState::Listen;
}

void TcpPcb::connect(Ipv4Address remote, uint16_t rport) {
  remote_ip = remote;
  remote_port = rport;
  active_open = true;
  snd_nxt = iss;
  snd_una = iss;
  state = TcpState::SynSent;
  emit_seg(TcpSegment::SYN);
}

void TcpPcb::close() {
  if (state == TcpState::Established) {
    state = TcpState::FinWait1;
    emit_seg(TcpSegment::FIN | TcpSegment::ACK);
  } else if (state == TcpState::CloseWait) {
    state = TcpState::LastAck;
    emit_seg(TcpSegment::FIN | TcpSegment::ACK);
  }
}

void TcpPcb::send_app(const Buffer& data) {
  if (state != TcpState::Established) {
    app_tx.insert(app_tx.end(), data.begin(), data.end());
    return;
  }
  Buffer remaining = data;
  while (!remaining.empty()) {
    std::size_t n = std::min<std::size_t>(remaining.size(), cc.mss);
    Buffer chunk(remaining.begin(), remaining.begin() + static_cast<std::ptrdiff_t>(n));
    remaining.erase(remaining.begin(), remaining.begin() + static_cast<std::ptrdiff_t>(n));
    emit_seg(TcpSegment::ACK | TcpSegment::PSH, chunk);
  }
}

Buffer TcpPcb::recv_app(std::size_t max_n) {
  Buffer out;
  while (!app_rx.empty() && out.size() < max_n) {
    out.push_back(app_rx.front());
    app_rx.pop_front();
  }
  return out;
}

void TcpPcb::on_segment(const TcpSegment& seg) {
  switch (state) {
    case TcpState::Listen:
      if (seg.flags & TcpSegment::SYN) {
        remote_port = seg.src_port;
        irs = seg.seq;
        rcv_nxt = irs + 1;
        snd_nxt = iss;
        snd_una = iss;
        state = TcpState::SynReceived;
        emit_seg(TcpSegment::SYN | TcpSegment::ACK);
      }
      break;

    case TcpState::SynSent:
      if ((seg.flags & TcpSegment::SYN) && (seg.flags & TcpSegment::ACK) && seg.ack == iss + 1) {
        irs = seg.seq;
        rcv_nxt = irs + 1;
        snd_una = seg.ack;
        state = TcpState::Established;
        emit_seg(TcpSegment::ACK);
        // flush queued app data
        if (!app_tx.empty()) {
          Buffer q(app_tx.begin(), app_tx.end());
          app_tx.clear();
          send_app(q);
        }
      }
      break;

    case TcpState::SynReceived:
      if ((seg.flags & TcpSegment::ACK) && seg.ack == iss + 1) {
        snd_una = seg.ack;
        state = TcpState::Established;
      }
      break;

    case TcpState::Established:
    case TcpState::FinWait1:
    case TcpState::FinWait2:
    case TcpState::CloseWait:
    case TcpState::LastAck:
      if (!seg.payload.empty()) {
        if (seg.seq == rcv_nxt) {
          app_rx.insert(app_rx.end(), seg.payload.begin(), seg.payload.end());
          rcv_nxt += static_cast<uint32_t>(seg.payload.size());
          emit_seg(TcpSegment::ACK);
        }
      }
      if (seg.flags & TcpSegment::ACK) {
        if (seg.ack > snd_una) {
          snd_una = seg.ack;
          cc.on_ack_new_data();
        } else if (seg.ack == snd_una) {
          cc.on_dup_ack();
        }
        if (state == TcpState::FinWait1 && seg.ack == snd_nxt) state = TcpState::FinWait2;
        if (state == TcpState::LastAck && seg.ack == snd_nxt) state = TcpState::Closed;
      }
      if (seg.flags & TcpSegment::FIN) {
        if (seg.seq == rcv_nxt || seg.seq + 1 == rcv_nxt) {
          if (seg.seq == rcv_nxt) rcv_nxt += 1;
          emit_seg(TcpSegment::ACK);
          if (state == TcpState::Established) state = TcpState::CloseWait;
          else if (state == TcpState::FinWait1 || state == TcpState::FinWait2)
            state = TcpState::TimeWait;
        }
      }
      break;

    default:
      break;
  }
}

}  // namespace netsim
