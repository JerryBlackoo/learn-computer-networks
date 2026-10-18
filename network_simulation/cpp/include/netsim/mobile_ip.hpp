#pragma once

#include "netsim/types.hpp"
#include "netsim/ipv4.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace netsim {

constexpr uint8_t IPPROTO_IPIP = 4; // IP-in-IP encapsulation protocol number (RFC 2003)

struct MobileBinding {
  Ipv4Address home_address;   // Permanent Home Address (HoA)
  Ipv4Address care_of_address;// Foreign Care-of Address (CoA)
  uint32_t lifetime_secs = 3600;
};

/**
 * RFC 5944 / RFC 3344 Mobile IP Home Agent (HA)
 * - Intercepts packets destined to Mobile Node's permanent Home Address
 * - Tunnels intercepted packets to the Care-of Address (CoA) using IP-in-IP encapsulation
 * - Maintains mobility binding table
 */
class HomeAgent {
 public:
  explicit HomeAgent(Ipv4Address agent_ip);

  Ipv4Address ip() const { return agent_ip_; }

  // Register or update binding for Mobile Node
  void register_binding(Ipv4Address home_addr, Ipv4Address care_of_addr, uint32_t lifetime = 3600);
  void deregister_binding(Ipv4Address home_addr);

  std::optional<Ipv4Address> lookup_care_of_address(Ipv4Address home_addr) const;
  bool is_registered(Ipv4Address home_addr) const;
  size_t active_bindings_count() const { return bindings_.size(); }

  // Intercept datagram destined to home_addr and encapsulate with IP-in-IP tunnel
  Ipv4Packet tunnel_packet(const Ipv4Packet& inner_datagram, Ipv4Address care_of_addr);

 private:
  Ipv4Address agent_ip_;
  struct IpHash {
    std::size_t operator()(const Ipv4Address& ip) const noexcept {
      return std::hash<uint32_t>{}(ip.host_order);
    }
  };
  std::unordered_map<Ipv4Address, MobileBinding, IpHash> bindings_;
};

/**
 * Mobile IP Foreign Agent (FA)
 * - Terminates IP-in-IP tunnel from Home Agent
 * - Decapsulates and delivers inner datagram to Mobile Node
 */
class ForeignAgent {
 public:
  explicit ForeignAgent(Ipv4Address agent_ip);

  Ipv4Address care_of_address() const { return agent_ip_; }

  // Decapsulates incoming IP-in-IP tunnel packet and retrieves original datagram
  std::optional<Ipv4Packet> decapsulate_tunnel(const Ipv4Packet& outer_datagram);

 private:
  Ipv4Address agent_ip_;
};

/**
 * Mobile Node (MN) roaming representation
 */
class MobileNode {
 public:
  MobileNode(Ipv4Address home_address, Ipv4Address home_agent_ip);

  Ipv4Address home_address() const { return home_address_; }
  Ipv4Address home_agent_ip() const { return home_agent_ip_; }
  std::optional<Ipv4Address> care_of_address() const { return care_of_address_; }

  bool is_roaming() const { return care_of_address_.has_value(); }

  void roam_to(Ipv4Address coa) { care_of_address_ = coa; }
  void return_home() { care_of_address_.reset(); }

  // Create an outbound packet to Correspondent Node (direct triangle routing)
  Ipv4Packet create_outbound_packet(Ipv4Address dst_cn, uint8_t proto, Buffer payload);

 private:
  Ipv4Address home_address_;
  Ipv4Address home_agent_ip_;
  std::optional<Ipv4Address> care_of_address_;
};

}  // namespace netsim

