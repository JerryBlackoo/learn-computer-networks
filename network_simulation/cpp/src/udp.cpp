#include "netsim/udp.hpp"

#include "netsim/checksum.hpp"

namespace netsim {

namespace {
Buffer udp_pseudo(Ipv4Address src, Ipv4Address dst, uint16_t udp_len, const Byte* udp,
                  std::size_t udp_bytes) {
  Buffer p(12 + udp_bytes);
  src.write_bytes(p.data());
  dst.write_bytes(p.data() + 4);
  p[8] = 0;
  p[9] = 17;
  write_u16_be(p.data() + 10, udp_len);
  std::copy(udp, udp + udp_bytes, p.begin() + 12);
  return p;
}
}  // namespace

Buffer UdpDatagram::serialize(Ipv4Address src, Ipv4Address dst) const {
  uint16_t len = static_cast<uint16_t>(8 + payload.size());
  Buffer out(len);
  write_u16_be(out.data(), src_port);
  write_u16_be(out.data() + 2, dst_port);
  write_u16_be(out.data() + 4, len);
  write_u16_be(out.data() + 6, 0);
  std::copy(payload.begin(), payload.end(), out.begin() + 8);
  Buffer pseudo = udp_pseudo(src, dst, len, out.data(), out.size());
  uint16_t csum = internet_checksum(pseudo);
  if (csum == 0) csum = 0xFFFF;
  write_u16_be(out.data() + 6, csum);
  return out;
}

bool UdpDatagram::parse(const Buffer& buf, Ipv4Address src, Ipv4Address dst, UdpDatagram& out,
                        bool verify_checksum) {
  if (buf.size() < 8) return false;
  out.src_port = read_u16_be(buf.data());
  out.dst_port = read_u16_be(buf.data() + 2);
  out.length = read_u16_be(buf.data() + 4);
  out.checksum = read_u16_be(buf.data() + 6);
  if (out.length < 8 || out.length > buf.size()) return false;
  if (verify_checksum && out.checksum != 0) {
    Buffer pseudo = udp_pseudo(src, dst, out.length, buf.data(), out.length);
    if (internet_checksum(pseudo) != 0) return false;
  }
  out.payload.assign(buf.begin() + 8, buf.begin() + out.length);
  return true;
}

}  // namespace netsim
