#pragma once

#include "netsim/types.hpp"

#include <cstdint>
#include <string>

namespace netsim {

constexpr uint8_t ICMP_TYPE_ECHO_REPLY = 0;
constexpr uint8_t ICMP_TYPE_DEST_UNREACHABLE = 3;
constexpr uint8_t ICMP_TYPE_ECHO_REQUEST = 8;
constexpr uint8_t ICMP_TYPE_TIME_EXCEEDED = 11;

/**
 * RFC 792 Internet Control Message Protocol (ICMP)
 */
struct IcmpMessage {
  uint8_t type = ICMP_TYPE_ECHO_REQUEST;
  uint8_t code = 0;
  uint16_t id = 0;
  uint16_t sequence = 0;
  Buffer payload;

  Buffer serialize() const;
  static bool parse(const Buffer& raw, IcmpMessage& out);

  static IcmpMessage make_echo_request(uint16_t id, uint16_t seq, Buffer payload = {});
  static IcmpMessage make_echo_reply(uint16_t id, uint16_t seq, Buffer payload = {});
  static IcmpMessage make_time_exceeded(Buffer original_ip_header_and_data);
  static IcmpMessage make_dest_unreachable(uint8_t code, Buffer original_ip_header_and_data);
};

}  // namespace netsim

