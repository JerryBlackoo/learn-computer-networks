#pragma once

#include "netsim/types.hpp"

namespace netsim {

/** Ethernet-style CRC-32 (poly 0xEDB88320, reflected). */
uint32_t crc32(const Byte* data, std::size_t len);
inline uint32_t crc32(const Buffer& buf) { return crc32(buf.data(), buf.size()); }

}  // namespace netsim
