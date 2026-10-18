#include "netsim/ethernet.hpp"

#include "netsim/crc.hpp"

namespace netsim {

Buffer EthernetFrame::serialize() const {
  Buffer out;
  out.reserve(14 + payload.size() + 4);
  out.insert(out.end(), dst.bytes.begin(), dst.bytes.end());
  out.insert(out.end(), src.bytes.begin(), src.bytes.end());
  Byte t[2];
  write_u16_be(t, ethertype);
  out.push_back(t[0]);
  out.push_back(t[1]);
  out.insert(out.end(), payload.begin(), payload.end());
  uint32_t fcs = crc32(out);
  Byte f[4];
  write_u32_be(f, fcs);
  out.insert(out.end(), f, f + 4);
  return out;
}

bool EthernetFrame::parse(const Buffer& raw, EthernetFrame& out) {
  if (raw.size() < 18) return false;
  Buffer body(raw.begin(), raw.end() - 4);
  uint32_t expect = read_u32_be(raw.data() + raw.size() - 4);
  if (crc32(body) != expect) return false;
  out.dst.bytes = {raw[0], raw[1], raw[2], raw[3], raw[4], raw[5]};
  out.src.bytes = {raw[6], raw[7], raw[8], raw[9], raw[10], raw[11]};
  out.ethertype = read_u16_be(raw.data() + 12);
  out.payload.assign(raw.begin() + 14, raw.end() - 4);
  return true;
}

}  // namespace netsim
