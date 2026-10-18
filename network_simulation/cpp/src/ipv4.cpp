#include "netsim/ipv4.hpp"

#include "netsim/checksum.hpp"

namespace netsim {

Buffer Ipv4Packet::serialize() const {
  Buffer out(20 + payload.size());
  out[0] = version_ihl;
  out[1] = tos;
  uint16_t tot = static_cast<uint16_t>(20 + payload.size());
  write_u16_be(out.data() + 2, tot);
  write_u16_be(out.data() + 4, id);
  write_u16_be(out.data() + 6, flags_frag);
  out[8] = ttl;
  out[9] = protocol;
  write_u16_be(out.data() + 10, 0);
  src.write_bytes(out.data() + 12);
  dst.write_bytes(out.data() + 16);
  uint16_t csum = internet_checksum(out.data(), 20);
  write_u16_be(out.data() + 10, csum);
  std::copy(payload.begin(), payload.end(), out.begin() + 20);
  return out;
}

bool Ipv4Packet::parse(const Buffer& buf, Ipv4Packet& out) {
  if (buf.size() < 20) return false;
  out.version_ihl = buf[0];
  if ((out.version_ihl >> 4) != 4) return false;
  int ihl = (out.version_ihl & 0x0F) * 4;
  if (ihl < 20 || static_cast<int>(buf.size()) < ihl) return false;
  Buffer hdr(buf.begin(), buf.begin() + ihl);
  if (internet_checksum(hdr) != 0) return false;
  out.tos = buf[1];
  out.total_length = read_u16_be(buf.data() + 2);
  out.id = read_u16_be(buf.data() + 4);
  out.flags_frag = read_u16_be(buf.data() + 6);
  out.ttl = buf[8];
  out.protocol = buf[9];
  out.header_checksum = read_u16_be(buf.data() + 10);
  out.src = Ipv4Address::from_bytes(buf.data() + 12);
  out.dst = Ipv4Address::from_bytes(buf.data() + 16);
  if (out.total_length > buf.size()) return false;
  out.payload.assign(buf.begin() + ihl, buf.begin() + out.total_length);
  return true;
}

}  // namespace netsim
