#include "netsim/checksum.hpp"

namespace netsim {

uint16_t internet_checksum(const Byte* data, std::size_t len, uint32_t initial) {
  uint32_t sum = initial;
  std::size_t i = 0;
  for (; i + 1 < len; i += 2) {
    sum += static_cast<uint32_t>((data[i] << 8) | data[i + 1]);
  }
  if (i < len) {
    sum += static_cast<uint32_t>(data[i] << 8);
  }
  while (sum >> 16) {
    sum = (sum & 0xFFFF) + (sum >> 16);
  }
  return static_cast<uint16_t>(~sum);
}

}  // namespace netsim
