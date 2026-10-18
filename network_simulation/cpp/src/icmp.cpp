#include "netsim/icmp.hpp"

#include "netsim/checksum.hpp"

namespace netsim {

Buffer IcmpMessage::serialize() const {
  Buffer out;
  out.reserve(8 + payload.size());
  out.push_back(type);
  out.push_back(code);
  out.push_back(0); // Checksum high (init 0)
  out.push_back(0); // Checksum low
  Byte id_bytes[2];
  write_u16_be(id_bytes, id);
  out.push_back(id_bytes[0]);
  out.push_back(id_bytes[1]);
  Byte seq_bytes[2];
  write_u16_be(seq_bytes, sequence);
  out.push_back(seq_bytes[0]);
  out.push_back(seq_bytes[1]);

  out.insert(out.end(), payload.begin(), payload.end());

  uint16_t csum = internet_checksum(out);
  Byte csum_bytes[2];
  write_u16_be(csum_bytes, csum);
  out[2] = csum_bytes[0];
  out[3] = csum_bytes[1];
  return out;
}

bool IcmpMessage::parse(const Buffer& raw, IcmpMessage& out) {
  if (raw.size() < 8) return false;
  if (internet_checksum(raw) != 0) return false; // Verify 1's complement sum

  out.type = raw[0];
  out.code = raw[1];
  out.id = read_u16_be(raw.data() + 4);
  out.sequence = read_u16_be(raw.data() + 6);
  out.payload.assign(raw.begin() + 8, raw.end());
  return true;
}

IcmpMessage IcmpMessage::make_echo_request(uint16_t id, uint16_t seq, Buffer payload) {
  IcmpMessage m;
  m.type = ICMP_TYPE_ECHO_REQUEST;
  m.code = 0;
  m.id = id;
  m.sequence = seq;
  m.payload = std::move(payload);
  return m;
}

IcmpMessage IcmpMessage::make_echo_reply(uint16_t id, uint16_t seq, Buffer payload) {
  IcmpMessage m;
  m.type = ICMP_TYPE_ECHO_REPLY;
  m.code = 0;
  m.id = id;
  m.sequence = seq;
  m.payload = std::move(payload);
  return m;
}

IcmpMessage IcmpMessage::make_time_exceeded(Buffer original_ip_header_and_data) {
  IcmpMessage m;
  m.type = ICMP_TYPE_TIME_EXCEEDED;
  m.code = 0; // TTL expired in transit
  m.id = 0;
  m.sequence = 0;
  m.payload = std::move(original_ip_header_and_data);
  return m;
}

IcmpMessage IcmpMessage::make_dest_unreachable(uint8_t code, Buffer original_ip_header_and_data) {
  IcmpMessage m;
  m.type = ICMP_TYPE_DEST_UNREACHABLE;
  m.code = code;
  m.id = 0;
  m.sequence = 0;
  m.payload = std::move(original_ip_header_and_data);
  return m;
}

}  // namespace netsim

