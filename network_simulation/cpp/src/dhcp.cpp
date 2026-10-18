#include "netsim/dhcp.hpp"

#include <stdexcept>

namespace netsim {

Buffer DhcpPacket::serialize() const {
  Buffer out;
  out.reserve(240);
  out.push_back(op);
  out.push_back(1); // htype: 10Mb Ethernet
  out.push_back(6); // hlen: 6 bytes MAC
  out.push_back(0); // hops: 0

  Byte xid_b[4];
  write_u32_be(xid_b, xid);
  out.insert(out.end(), xid_b, xid_b + 4);

  // secs (2B) + flags (2B) = 4B zeros
  out.insert(out.end(), 4, 0);

  // ciaddr (4B)
  Byte ip_b[4];
  ciaddr.write_bytes(ip_b);
  out.insert(out.end(), ip_b, ip_b + 4);

  // yiaddr (4B)
  yiaddr.write_bytes(ip_b);
  out.insert(out.end(), ip_b, ip_b + 4);

  // siaddr (4B)
  siaddr.write_bytes(ip_b);
  out.insert(out.end(), ip_b, ip_b + 4);

  // giaddr (4B) zeros
  out.insert(out.end(), 4, 0);

  // chaddr (16B, client MAC + 10B padding)
  out.insert(out.end(), chaddr.bytes.begin(), chaddr.bytes.end());
  out.insert(out.end(), 10, 0);

  // Magic Cookie (RFC 1497: 99, 130, 83, 99)
  out.push_back(99);
  out.push_back(130);
  out.push_back(83);
  out.push_back(99);

  // Option 53: DHCP Message Type (len 1)
  out.push_back(53);
  out.push_back(1);
  out.push_back(static_cast<uint8_t>(type));

  // Option 1: Subnet Mask (len 4)
  if (subnet_mask.host_order != 0) {
    out.push_back(1);
    out.push_back(4);
    subnet_mask.write_bytes(ip_b);
    out.insert(out.end(), ip_b, ip_b + 4);
  }

  // Option 3: Router (len 4)
  if (router.host_order != 0) {
    out.push_back(3);
    out.push_back(4);
    router.write_bytes(ip_b);
    out.insert(out.end(), ip_b, ip_b + 4);
  }

  // Option 6: DNS Server (len 4)
  if (dns_server.host_order != 0) {
    out.push_back(6);
    out.push_back(4);
    dns_server.write_bytes(ip_b);
    out.insert(out.end(), ip_b, ip_b + 4);
  }

  // Option 51: Lease Time (len 4)
  out.push_back(51);
  out.push_back(4);
  write_u32_be(ip_b, lease_time_secs);
  out.insert(out.end(), ip_b, ip_b + 4);

  // Option 255: End
  out.push_back(255);

  return out;
}

bool DhcpPacket::parse(const Buffer& raw, DhcpPacket& out) {
  if (raw.size() < 44) return false;

  out.op = raw[0];
  out.xid = read_u32_be(raw.data() + 4);
  out.ciaddr = Ipv4Address::from_bytes(raw.data() + 12);
  out.yiaddr = Ipv4Address::from_bytes(raw.data() + 16);
  out.siaddr = Ipv4Address::from_bytes(raw.data() + 20);

  std::copy(raw.begin() + 28, raw.begin() + 34, out.chaddr.bytes.begin());

  // Parse Options after offset 44 (if magic cookie present)
  size_t idx = 44;
  if (raw.size() >= 48 && raw[44] == 99 && raw[45] == 130 && raw[46] == 83 && raw[47] == 99) {
    idx = 48; // Skip magic cookie
  }

  while (idx < raw.size()) {
    uint8_t opt = raw[idx++];
    if (opt == 0) continue; // Pad
    if (opt == 255) break;  // End option

    if (idx >= raw.size()) break;
    uint8_t len = raw[idx++];
    if (idx + len > raw.size()) break;

    if (opt == 53 && len == 1) {
      out.type = static_cast<DhcpMessageType>(raw[idx]);
    } else if (opt == 1 && len == 4) {
      out.subnet_mask = Ipv4Address::from_bytes(raw.data() + idx);
    } else if (opt == 3 && len == 4) {
      out.router = Ipv4Address::from_bytes(raw.data() + idx);
    } else if (opt == 6 && len == 4) {
      out.dns_server = Ipv4Address::from_bytes(raw.data() + idx);
    } else if (opt == 51 && len == 4) {
      out.lease_time_secs = read_u32_be(raw.data() + idx);
    }

    idx += len;
  }

  return true;
}

// ============================================================================
// DhcpServer
// ============================================================================
DhcpServer::DhcpServer(Ipv4Address server_ip, Ipv4Address pool_start, int pool_size,
                       Ipv4Address mask, Ipv4Address gateway, Ipv4Address dns)
    : server_ip_(server_ip),
      pool_start_(pool_start),
      pool_size_(pool_size),
      mask_(mask),
      gateway_(gateway),
      dns_(dns) {}

std::optional<DhcpPacket> DhcpServer::handle_packet(const DhcpPacket& req) {
  if (req.type == DhcpMessageType::DISCOVER) {
    // Generate DHCPOFFER
    Ipv4Address allocated_ip;
    auto it = leases_.find(req.chaddr);
    if (it != leases_.end()) {
      allocated_ip = it->second;
    } else {
      allocated_ip.host_order = pool_start_.host_order + next_offset_;
      next_offset_ = (next_offset_ + 1) % pool_size_;
      leases_[req.chaddr] = allocated_ip;
    }

    DhcpPacket offer;
    offer.op = 2; // BOOTREPLY
    offer.xid = req.xid;
    offer.yiaddr = allocated_ip;
    offer.siaddr = server_ip_;
    offer.chaddr = req.chaddr;
    offer.type = DhcpMessageType::OFFER;
    offer.subnet_mask = mask_;
    offer.router = gateway_;
    offer.dns_server = dns_;
    offer.lease_time_secs = 86400;
    return offer;
  }

  if (req.type == DhcpMessageType::REQUEST) {
    // Generate DHCPACK
    auto it = leases_.find(req.chaddr);
    Ipv4Address confirmed_ip = (it != leases_.end()) ? it->second : req.yiaddr;

    DhcpPacket ack;
    ack.op = 2; // BOOTREPLY
    ack.xid = req.xid;
    ack.yiaddr = confirmed_ip;
    ack.siaddr = server_ip_;
    ack.chaddr = req.chaddr;
    ack.type = DhcpMessageType::ACK;
    ack.subnet_mask = mask_;
    ack.router = gateway_;
    ack.dns_server = dns_;
    ack.lease_time_secs = 86400;
    return ack;
  }

  return std::nullopt;
}

std::optional<Ipv4Address> DhcpServer::get_lease(const MacAddress& mac) const {
  auto it = leases_.find(mac);
  if (it != leases_.end()) return it->second;
  return std::nullopt;
}

// ============================================================================
// DhcpClient
// ============================================================================
DhcpClient::DhcpClient(MacAddress mac) : mac_(mac) {}

DhcpPacket DhcpClient::create_discover(uint32_t xid) {
  current_xid_ = xid;
  DhcpPacket p;
  p.op = 1; // BOOTREQUEST
  p.xid = current_xid_;
  p.chaddr = mac_;
  p.type = DhcpMessageType::DISCOVER;
  state_ = State::SELECTING;
  return p;
}

std::optional<DhcpPacket> DhcpClient::handle_offer(const DhcpPacket& offer) {
  if (state_ != State::SELECTING || offer.xid != current_xid_ ||
      offer.type != DhcpMessageType::OFFER) {
    return std::nullopt;
  }

  offered_ip_ = offer.yiaddr;
  mask_ = offer.subnet_mask;
  gateway_ = offer.router;
  dns_ = offer.dns_server;

  // Build DHCPREQUEST
  DhcpPacket req;
  req.op = 1; // BOOTREQUEST
  req.xid = current_xid_;
  req.yiaddr = offered_ip_;
  req.siaddr = offer.siaddr;
  req.chaddr = mac_;
  req.type = DhcpMessageType::REQUEST;
  state_ = State::REQUESTING;
  return req;
}

bool DhcpClient::handle_ack(const DhcpPacket& ack) {
  if (state_ != State::REQUESTING || ack.xid != current_xid_ ||
      ack.type != DhcpMessageType::ACK) {
    return false;
  }

  assigned_ip_ = ack.yiaddr;
  mask_ = ack.subnet_mask;
  gateway_ = ack.router;
  dns_ = ack.dns_server;
  state_ = State::BOUND;
  return true;
}

}  // namespace netsim

