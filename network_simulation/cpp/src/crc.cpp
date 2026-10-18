#include "netsim/crc.hpp"

namespace netsim {

namespace {
uint32_t table[256];
bool ready = false;

void init() {
  for (uint32_t i = 0; i < 256; ++i) {
    uint32_t c = i;
    for (int j = 0; j < 8; ++j) {
      c = (c & 1) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
    }
    table[i] = c;
  }
  ready = true;
}
}  // namespace

uint32_t crc32(const Byte* data, std::size_t len) {
  if (!ready) init();
  uint32_t c = 0xFFFFFFFFu;
  for (std::size_t i = 0; i < len; ++i) {
    c = table[(c ^ data[i]) & 0xFF] ^ (c >> 8);
  }
  return c ^ 0xFFFFFFFFu;
}

}  // namespace netsim
