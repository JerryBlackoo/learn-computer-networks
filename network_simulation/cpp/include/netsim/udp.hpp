#pragma once

#include "netsim/types.hpp"

namespace netsim {

struct UdpDatagram {
  uint16_t src_port = 0;
  uint16_t dst_port = 0;
  uint16_t length = 0;
  uint16_t checksum = 0;
  Buffer payload;

  Buffer serialize(Ipv4Address src, Ipv4Address dst) const;
  static bool parse(const Buffer& buf, Ipv4Address src, Ipv4Address dst, UdpDatagram& out,
                    bool verify_checksum = true);
};

}  // namespace netsim
