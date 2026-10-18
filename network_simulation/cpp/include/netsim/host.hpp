#pragma once

#include "netsim/arp.hpp"
#include "netsim/dns.hpp"
#include "netsim/ethernet.hpp"
#include "netsim/http.hpp"
#include "netsim/ipv4.hpp"
#include "netsim/phy.hpp"
#include "netsim/route.hpp"
#include "netsim/tcp.hpp"
#include "netsim/udp.hpp"

#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace netsim {

struct Iface {
  std::string name;
  MacAddress mac;
  Ipv4Address ip;
  uint8_t prefix_len = 24;
  PhyLink* tx = nullptr;  // frames leaving this host
  PhyLink* rx = nullptr;  // frames arriving at this host
};

/**
 * Five-layer teaching host:
 * App (DNS/HTTP) -> Transport (TCP/UDP) -> Network (IP/LPM/ARP) -> Link (Eth/CRC) -> Phy.
 */
class Host {
 public:
  explicit Host(std::string name);

  void add_iface(Iface iface);
  void add_route(RouteEntry e);
  DnsResolver& dns() { return dns_; }
  const std::string& name() const { return name_; }

  /** Drive one RX frame from an iface if present. */
  void poll();

  /** TCP helpers for e2e demos. */
  TcpPcb& tcp() { return tcp_; }
  void bind_tcp_emit();

  /** Send raw IPv4 payload to dst (looks up route + ARP). */
  bool send_ip(Ipv4Address dst, uint8_t proto, const Buffer& payload);

  /** HTTP GET over established TCP (caller manages connect). */
  bool http_exchange(const HttpRequest& req, HttpResponse& resp, int max_polls = 100);

  /** Serve one HTTP response when request arrives on TCP rx. */
  void serve_http_once(const std::function<HttpResponse(const HttpRequest&)>& handler);

  ArpCache& arp() { return arp_; }
  RouteTable& routes() { return routes_; }
  std::vector<Iface>& ifaces() { return ifaces_; }

 private:
  struct PendingIp {
    std::string iface_name;
    Ipv4Address next_hop;
    Ipv4Address dst;
    uint8_t proto = 0;
    Buffer payload;
  };

  void on_ethernet(Iface& iface, const EthernetFrame& frame);
  void on_arp(Iface& iface, const Buffer& payload);
  void on_ipv4(Iface& iface, const Buffer& payload);
  void on_udp(const Ipv4Packet& ip, const Buffer& payload);
  void on_tcp(const Ipv4Packet& ip, const Buffer& payload);
  bool resolve_mac(Iface& iface, Ipv4Address next_hop, MacAddress& mac);
  bool emit_ethernet_ip(Iface& iface, MacAddress dst_mac, Ipv4Address dst, uint8_t proto,
                        const Buffer& payload);
  void flush_pending_arp(Ipv4Address ip);
  Iface* find_iface(const std::string& name);
  Iface* iface_for_route(const RouteEntry& e);

  std::string name_;
  std::vector<Iface> ifaces_;
  RouteTable routes_;
  ArpCache arp_;
  DnsResolver dns_;
  TcpPcb tcp_;
  uint16_t ip_id_ = 1;
  std::vector<PendingIp> pending_arp_;
};

}  // namespace netsim
