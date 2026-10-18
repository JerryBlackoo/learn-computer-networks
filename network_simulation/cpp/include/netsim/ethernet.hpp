#pragma once

#include "netsim/types.hpp"

namespace netsim {

constexpr uint16_t ETHERTYPE_IPV4 = 0x0800;
constexpr uint16_t ETHERTYPE_ARP = 0x0806;

struct EthernetFrame {
  MacAddress dst;
  MacAddress src;
  uint16_t ethertype = ETHERTYPE_IPV4;
  Buffer payload;

  Buffer serialize() const;           // includes FCS
  static bool parse(const Buffer& raw, EthernetFrame& out);  // verifies CRC
};

}  // namespace netsim
