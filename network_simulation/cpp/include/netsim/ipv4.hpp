#pragma once

#include "netsim/types.hpp"

namespace netsim {

constexpr uint8_t IPPROTO_TCP = 6;
constexpr uint8_t IPPROTO_UDP = 17;
constexpr uint8_t IPPROTO_ICMP = 1;

struct Ipv4Packet {
  uint8_t version_ihl = 0x45;
  uint8_t tos = 0;
  uint16_t total_length = 0;
  uint16_t id = 0;
  uint16_t flags_frag = 0;
  uint8_t ttl = 64;
  uint8_t protocol = IPPROTO_UDP;
  uint16_t header_checksum = 0;
  Ipv4Address src;
  Ipv4Address dst;
  Buffer payload;

  Buffer serialize() const;  // fills length + checksum
  static bool parse(const Buffer& buf, Ipv4Packet& out);
};

}  // namespace netsim
