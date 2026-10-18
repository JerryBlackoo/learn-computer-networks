#include "netsim/mobile_ip.hpp"

namespace netsim {

// ============================================================================
// HomeAgent
// ============================================================================
HomeAgent::HomeAgent(Ipv4Address agent_ip) : agent_ip_(agent_ip) {}

void HomeAgent::register_binding(Ipv4Address home_addr, Ipv4Address care_of_addr,
                                uint32_t lifetime) {
  MobileBinding b;
  b.home_address = home_addr;
  b.care_of_address = care_of_addr;
  b.lifetime_secs = lifetime;
  bindings_[home_addr] = b;
}

void HomeAgent::deregister_binding(Ipv4Address home_addr) {
  bindings_.erase(home_addr);
}

std::optional<Ipv4Address> HomeAgent::lookup_care_of_address(Ipv4Address home_addr) const {
  auto it = bindings_.find(home_addr);
  if (it != bindings_.end()) {
    return it->second.care_of_address;
  }
  return std::nullopt;
}

bool HomeAgent::is_registered(Ipv4Address home_addr) const {
  return bindings_.find(home_addr) != bindings_.end();
}

Ipv4Packet HomeAgent::tunnel_packet(const Ipv4Packet& inner_datagram, Ipv4Address care_of_addr) {
  // Encapsulate original datagram inside an outer IP packet (IP-in-IP, protocol 4)
  Buffer serialized_inner = inner_datagram.serialize();

  Ipv4Packet outer;
  outer.src = agent_ip_;
  outer.dst = care_of_addr;
  outer.protocol = IPPROTO_IPIP; // Protocol 4 (IP-in-IP encapsulation)
  outer.ttl = 64;
  outer.payload = std::move(serialized_inner);
  outer.total_length = static_cast<uint16_t>(20 + outer.payload.size());

  return outer;
}

// ============================================================================
// ForeignAgent
// ============================================================================
ForeignAgent::ForeignAgent(Ipv4Address agent_ip) : agent_ip_(agent_ip) {}

std::optional<Ipv4Packet> ForeignAgent::decapsulate_tunnel(const Ipv4Packet& outer_datagram) {
  if (outer_datagram.protocol != IPPROTO_IPIP) {
    return std::nullopt;
  }
  Ipv4Packet inner;
  if (!Ipv4Packet::parse(outer_datagram.payload, inner)) {
    return std::nullopt;
  }
  return inner;
}

// ============================================================================
// MobileNode
// ============================================================================
MobileNode::MobileNode(Ipv4Address home_address, Ipv4Address home_agent_ip)
    : home_address_(home_address), home_agent_ip_(home_agent_ip) {}

Ipv4Packet MobileNode::create_outbound_packet(Ipv4Address dst_cn, uint8_t proto, Buffer payload) {
  // Direct Triangle Routing: Source is always the permanent Home Address!
  Ipv4Packet pkt;
  pkt.src = home_address_;
  pkt.dst = dst_cn;
  pkt.protocol = proto;
  pkt.ttl = 64;
  pkt.payload = std::move(payload);
  pkt.total_length = static_cast<uint16_t>(20 + pkt.payload.size());
  return pkt;
}

}  // namespace netsim
