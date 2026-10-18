#pragma once

#include "netsim/types.hpp"

namespace netsim {

/** RFC 1071 Internet checksum (one's complement of 16-bit words). */
uint16_t internet_checksum(const Byte* data, std::size_t len, uint32_t initial = 0);

inline uint16_t internet_checksum(const Buffer& buf, uint32_t initial = 0) {
  return internet_checksum(buf.data(), buf.size(), initial);
}

}  // namespace netsim
