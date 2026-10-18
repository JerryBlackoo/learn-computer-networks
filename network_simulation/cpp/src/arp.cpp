#include "netsim/arp.hpp"

#include <algorithm>

namespace netsim {

Buffer ArpPacket::serialize() const {
  Buffer out(28);
  write_u16_be(out.data(), htype);
  write_u16_be(out.data() + 2, ptype);
  out[4] = hlen;
  out[5] = plen;
  write_u16_be(out.data() + 6, oper);
  std::copy(sha.bytes.begin(), sha.bytes.end(), out.begin() + 8);
  spa.write_bytes(out.data() + 14);
  std::copy(tha.bytes.begin(), tha.bytes.end(), out.begin() + 18);
  tpa.write_bytes(out.data() + 24);
  return out;
}

bool ArpPacket::parse(const Buffer& buf, ArpPacket& out) {
  if (buf.size() < 28) return false;
  out.htype = read_u16_be(buf.data());
  out.ptype = read_u16_be(buf.data() + 2);
  out.hlen = buf[4];
  out.plen = buf[5];
  out.oper = read_u16_be(buf.data() + 6);
  std::copy(buf.begin() + 8, buf.begin() + 14, out.sha.bytes.begin());
  out.spa = Ipv4Address::from_bytes(buf.data() + 14);
  std::copy(buf.begin() + 18, buf.begin() + 24, out.tha.bytes.begin());
  out.tpa = Ipv4Address::from_bytes(buf.data() + 24);
  return true;
}

void ArpCache::put(Ipv4Address ip, MacAddress mac) { table_[{ip.host_order}] = mac; }

std::optional<MacAddress> ArpCache::lookup(Ipv4Address ip) const {
  auto it = table_.find({ip.host_order});
  if (it == table_.end()) return std::nullopt;
  return it->second;
}

}  // namespace netsim
