#include "netsim/types.hpp"

#include <cstdio>
#include <sstream>
#include <stdexcept>

namespace netsim {

MacAddress MacAddress::from_string(const std::string& s) {
  MacAddress m;
  unsigned b[6];
  if (std::sscanf(s.c_str(), "%x:%x:%x:%x:%x:%x", &b[0], &b[1], &b[2], &b[3], &b[4], &b[5]) != 6) {
    throw std::invalid_argument("bad MAC: " + s);
  }
  for (int i = 0; i < 6; ++i) m.bytes[i] = static_cast<Byte>(b[i]);
  return m;
}

std::string MacAddress::to_string() const {
  char buf[32];
  std::snprintf(buf, sizeof(buf), "%02x:%02x:%02x:%02x:%02x:%02x", bytes[0], bytes[1], bytes[2],
                bytes[3], bytes[4], bytes[5]);
  return buf;
}

Ipv4Address Ipv4Address::from_string(const std::string& s) {
  unsigned a, b, c, d;
  if (std::sscanf(s.c_str(), "%u.%u.%u.%u", &a, &b, &c, &d) != 4 || a > 255 || b > 255 || c > 255 ||
      d > 255) {
    throw std::invalid_argument("bad IPv4: " + s);
  }
  Ipv4Address ip;
  ip.host_order = (a << 24) | (b << 16) | (c << 8) | d;
  return ip;
}

Ipv4Address Ipv4Address::from_bytes(const Byte* p) {
  Ipv4Address ip;
  ip.host_order = read_u32_be(p);
  return ip;
}

void Ipv4Address::write_bytes(Byte* p) const { write_u32_be(p, host_order); }

std::string Ipv4Address::to_string() const {
  return std::to_string((host_order >> 24) & 0xFF) + "." + std::to_string((host_order >> 16) & 0xFF) +
         "." + std::to_string((host_order >> 8) & 0xFF) + "." + std::to_string(host_order & 0xFF);
}

}  // namespace netsim
