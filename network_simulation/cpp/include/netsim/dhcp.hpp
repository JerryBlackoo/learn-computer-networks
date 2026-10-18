#pragma once

#include "netsim/types.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace netsim {

enum class DhcpMessageType : uint8_t {
  DISCOVER = 1,
  OFFER    = 2,
  REQUEST  = 3,
  DECLINE  = 4,
  ACK      = 5,
  NAK      = 6,
  RELEASE  = 7
};

/**
 * RFC 2131 Dynamic Host Configuration Protocol (DHCP) Packet
 */
struct DhcpPacket {
  uint8_t op = 1; // 1 = BOOTREQUEST (client to server), 2 = BOOTREPLY (server to client)
  uint32_t xid = 0x12345678; // Transaction ID
  Ipv4Address ciaddr; // Client IP (if known)
  Ipv4Address yiaddr; // "Your" (client) IP allocated by server
  Ipv4Address siaddr; // Server IP
  MacAddress chaddr;  // Client hardware MAC address

  DhcpMessageType type = DhcpMessageType::DISCOVER;
  Ipv4Address subnet_mask;
  Ipv4Address router;
  Ipv4Address dns_server;
  uint32_t lease_time_secs = 86400;

  Buffer serialize() const;
  static bool parse(const Buffer& raw, DhcpPacket& out);
};

struct MacHash {
  std::size_t operator()(const MacAddress& m) const noexcept {
    std::size_t h = 0;
    for (Byte b : m.bytes) {
      h = (h * 31) + b;
    }
    return h;
  }
};

/**
 * Teaching DHCP Server managing an IP address pool
 */
class DhcpServer {
 public:
  DhcpServer(Ipv4Address server_ip, Ipv4Address pool_start, int pool_size,
             Ipv4Address mask, Ipv4Address gateway, Ipv4Address dns);

  std::optional<DhcpPacket> handle_packet(const DhcpPacket& req);

  size_t active_lease_count() const { return leases_.size(); }
  std::optional<Ipv4Address> get_lease(const MacAddress& mac) const;

 private:
  Ipv4Address server_ip_;
  Ipv4Address pool_start_;
  int pool_size_;
  Ipv4Address mask_;
  Ipv4Address gateway_;
  Ipv4Address dns_;

  uint32_t next_offset_ = 0;
  std::unordered_map<MacAddress, Ipv4Address, MacHash> leases_;
};

/**
 * Teaching DHCP Client state machine:
 * INIT -> DISCOVER -> SELECTING (OFFER) -> REQUESTING (REQUEST) -> BOUND (ACK)
 */
class DhcpClient {
 public:
  enum class State {
    INIT,
    SELECTING,
    REQUESTING,
    BOUND
  };

  explicit DhcpClient(MacAddress mac);

  State state() const { return state_; }
  Ipv4Address ip() const { return assigned_ip_; }
  Ipv4Address mask() const { return mask_; }
  Ipv4Address gateway() const { return gateway_; }
  Ipv4Address dns() const { return dns_; }

  DhcpPacket create_discover(uint32_t xid = 0x87654321);
  std::optional<DhcpPacket> handle_offer(const DhcpPacket& offer);
  bool handle_ack(const DhcpPacket& ack);

 private:
  MacAddress mac_;
  State state_ = State::INIT;
  uint32_t current_xid_ = 0;

  Ipv4Address offered_ip_;
  Ipv4Address assigned_ip_;
  Ipv4Address mask_;
  Ipv4Address gateway_;
  Ipv4Address dns_;
};

}  // namespace netsim
