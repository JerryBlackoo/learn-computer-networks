#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace netsim {

using Byte = uint8_t;
using Buffer = std::vector<Byte>;

struct MacAddress {
  std::array<Byte, 6> bytes{};

  static MacAddress broadcast() {
    MacAddress m;
    m.bytes.fill(0xFF);
    return m;
  }

  static MacAddress from_string(const std::string& s);

  std::string to_string() const;
  bool operator==(const MacAddress& o) const { return bytes == o.bytes; }
  bool operator!=(const MacAddress& o) const { return !(*this == o); }
};

struct Ipv4Address {
  uint32_t host_order = 0;

  static Ipv4Address from_string(const std::string& s);
  static Ipv4Address from_bytes(const Byte* p);
  void write_bytes(Byte* p) const;
  std::string to_string() const;
  bool operator==(const Ipv4Address& o) const { return host_order == o.host_order; }
  bool operator!=(const Ipv4Address& o) const { return !(*this == o); }
};

inline uint16_t read_u16_be(const Byte* p) {
  return static_cast<uint16_t>((p[0] << 8) | p[1]);
}

inline uint32_t read_u32_be(const Byte* p) {
  return (static_cast<uint32_t>(p[0]) << 24) | (static_cast<uint32_t>(p[1]) << 16) |
         (static_cast<uint32_t>(p[2]) << 8) | static_cast<uint32_t>(p[3]);
}

inline void write_u16_be(Byte* p, uint16_t v) {
  p[0] = static_cast<Byte>((v >> 8) & 0xFF);
  p[1] = static_cast<Byte>(v & 0xFF);
}

inline void write_u32_be(Byte* p, uint32_t v) {
  p[0] = static_cast<Byte>((v >> 24) & 0xFF);
  p[1] = static_cast<Byte>((v >> 16) & 0xFF);
  p[2] = static_cast<Byte>((v >> 8) & 0xFF);
  p[3] = static_cast<Byte>(v & 0xFF);
}

}  // namespace netsim
