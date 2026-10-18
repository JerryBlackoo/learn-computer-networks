#pragma once

#include "netsim/types.hpp"

#include <optional>
#include <unordered_map>

namespace netsim {

struct ArpPacket {
  uint16_t htype = 1;  // Ethernet
  uint16_t ptype = 0x0800;
  uint8_t hlen = 6;
  uint8_t plen = 4;
  uint16_t oper = 1;  // 1 request, 2 reply
  MacAddress sha;
  Ipv4Address spa;
  MacAddress tha;
  Ipv4Address tpa;

  Buffer serialize() const;
  static bool parse(const Buffer& buf, ArpPacket& out);
};

class ArpCache {
 public:
  void put(Ipv4Address ip, MacAddress mac);
  std::optional<MacAddress> lookup(Ipv4Address ip) const;

 private:
  struct Key {
    uint32_t v;
    bool operator==(const Key& o) const { return v == o.v; }
  };
  struct Hash {
    std::size_t operator()(const Key& k) const { return std::hash<uint32_t>{}(k.v); }
  };
  std::unordered_map<Key, MacAddress, Hash> table_;
};

}  // namespace netsim
