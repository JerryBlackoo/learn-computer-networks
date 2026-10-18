#include "netsim/host.hpp"

#include <cstring>

namespace netsim {

Host::Host(std::string name) : name_(std::move(name)) {}

void Host::add_iface(Iface iface) { ifaces_.push_back(std::move(iface)); }

void Host::add_route(RouteEntry e) { routes_.add(std::move(e)); }

Iface* Host::find_iface(const std::string& name) {
  for (auto& i : ifaces_) {
    if (i.name == name) return &i;
  }
  return nullptr;
}

Iface* Host::iface_for_route(const RouteEntry& e) { return find_iface(e.iface); }

void Host::bind_tcp_emit() {
  tcp_.emit = [this](const TcpSegment& seg) {
    Buffer payload = seg.serialize(tcp_.local_ip, tcp_.remote_ip);
    send_ip(tcp_.remote_ip, IPPROTO_TCP, payload);
  };
}

bool Host::resolve_mac(Iface& iface, Ipv4Address next_hop, MacAddress& mac) {
  if (auto hit = arp_.lookup(next_hop)) {
    mac = *hit;
    return true;
  }
  ArpPacket req;
  req.oper = 1;
  req.sha = iface.mac;
  req.spa = iface.ip;
  req.tha = MacAddress{};
  req.tpa = next_hop;
  EthernetFrame frame;
  frame.dst = MacAddress::broadcast();
  frame.src = iface.mac;
  frame.ethertype = ETHERTYPE_ARP;
  frame.payload = req.serialize();
  if (iface.tx) iface.tx->send(frame.serialize());
  return false;
}

bool Host::emit_ethernet_ip(Iface& iface, MacAddress dst_mac, Ipv4Address dst, uint8_t proto,
                            const Buffer& payload) {
  Ipv4Packet ip;
  ip.id = ip_id_++;
  ip.protocol = proto;
  ip.src = iface.ip;
  ip.dst = dst;
  ip.payload = payload;
  if (proto == IPPROTO_TCP) tcp_.local_ip = iface.ip;

  EthernetFrame frame;
  frame.dst = dst_mac;
  frame.src = iface.mac;
  frame.ethertype = ETHERTYPE_IPV4;
  frame.payload = ip.serialize();
  if (!iface.tx) return false;
  iface.tx->send(frame.serialize());
  return true;
}

void Host::flush_pending_arp(Ipv4Address ip) {
  auto it = pending_arp_.begin();
  while (it != pending_arp_.end()) {
    if (it->next_hop != ip) {
      ++it;
      continue;
    }
    Iface* iface = find_iface(it->iface_name);
    auto mac = arp_.lookup(ip);
    if (iface && mac) {
      emit_ethernet_ip(*iface, *mac, it->dst, it->proto, it->payload);
    }
    it = pending_arp_.erase(it);
  }
}

bool Host::send_ip(Ipv4Address dst, uint8_t proto, const Buffer& payload) {
  auto route = routes_.longest_prefix_match(dst);
  if (!route) return false;
  Iface* iface = iface_for_route(*route);
  if (!iface || !iface->tx) return false;

  Ipv4Address next = route->next_hop.host_order == 0 ? dst : route->next_hop;
  MacAddress dst_mac;
  if (!resolve_mac(*iface, next, dst_mac)) {
    pending_arp_.push_back(PendingIp{iface->name, next, dst, proto, payload});
    return true;  // ARP in flight
  }
  return emit_ethernet_ip(*iface, dst_mac, dst, proto, payload);
}

void Host::poll() {
  for (auto& iface : ifaces_) {
    if (!iface.rx) continue;
    Buffer raw;
    while (iface.rx->receive(raw)) {
      EthernetFrame frame;
      if (!EthernetFrame::parse(raw, frame)) continue;
      on_ethernet(iface, frame);
    }
  }
}

void Host::on_ethernet(Iface& iface, const EthernetFrame& frame) {
  if (frame.ethertype == ETHERTYPE_ARP) {
    on_arp(iface, frame.payload);
  } else if (frame.ethertype == ETHERTYPE_IPV4) {
    on_ipv4(iface, frame.payload);
  }
}

void Host::on_arp(Iface& iface, const Buffer& payload) {
  ArpPacket pkt;
  if (!ArpPacket::parse(payload, pkt)) return;
  arp_.put(pkt.spa, pkt.sha);
  flush_pending_arp(pkt.spa);
  if (pkt.oper == 1 && pkt.tpa == iface.ip) {
    ArpPacket reply = pkt;
    reply.oper = 2;
    reply.tha = pkt.sha;
    reply.tpa = pkt.spa;
    reply.sha = iface.mac;
    reply.spa = iface.ip;
    EthernetFrame frame;
    frame.dst = pkt.sha;
    frame.src = iface.mac;
    frame.ethertype = ETHERTYPE_ARP;
    frame.payload = reply.serialize();
    if (iface.tx) iface.tx->send(frame.serialize());
  }
}

void Host::on_ipv4(Iface& /*iface*/, const Buffer& payload) {
  Ipv4Packet ip;
  if (!Ipv4Packet::parse(payload, ip)) return;
  // Accept if destined to any local iface IP
  bool for_us = false;
  for (const auto& i : ifaces_) {
    if (i.ip == ip.dst) for_us = true;
  }
  if (!for_us) return;
  if (ip.protocol == IPPROTO_UDP) on_udp(ip, ip.payload);
  else if (ip.protocol == IPPROTO_TCP) on_tcp(ip, ip.payload);
}

void Host::on_udp(const Ipv4Packet& ip, const Buffer& payload) {
  UdpDatagram udp;
  if (!UdpDatagram::parse(payload, ip.src, ip.dst, udp)) return;
  if (udp.dst_port == 53) {
    std::string qname;
    if (DnsResolver::decode_query(udp.payload, qname)) {
      auto ans = dns_.resolve(qname);
      if (!ans) return;
      UdpDatagram resp;
      resp.src_port = 53;
      resp.dst_port = udp.src_port;
      resp.payload = DnsResolver::encode_response(qname, *ans);
      Buffer enc = resp.serialize(ip.dst, ip.src);
      // find iface with ip.dst
      for (auto& i : ifaces_) {
        if (i.ip == ip.dst) {
          // seed ARP from reverse path
          send_ip(ip.src, IPPROTO_UDP, enc);
          break;
        }
      }
    }
  }
}

void Host::on_tcp(const Ipv4Packet& ip, const Buffer& payload) {
  TcpSegment seg;
  if (!TcpSegment::parse(payload, ip.src, ip.dst, seg)) return;
  tcp_.remote_ip = ip.src;
  tcp_.local_ip = ip.dst;
  if (tcp_.local_port == 0) tcp_.local_port = seg.dst_port;
  tcp_.on_segment(seg);
}

bool Host::http_exchange(const HttpRequest& req, HttpResponse& resp, int max_polls) {
  std::string s = req.serialize();
  Buffer data(s.begin(), s.end());
  tcp_.send_app(data);
  std::string acc;
  for (int i = 0; i < max_polls; ++i) {
    poll();
    Buffer chunk = tcp_.recv_app();
    acc.append(chunk.begin(), chunk.end());
    if (acc.find("\r\n\r\n") != std::string::npos) {
      // wait until Content-Length satisfied or simple complete
      if (HttpResponse::parse(acc, resp)) {
        auto cl = acc.find("Content-Length:");
        if (cl != std::string::npos) {
          // body already in parse
          return true;
        }
        return true;
      }
    }
  }
  return HttpResponse::parse(acc, resp);
}

void Host::serve_http_once(const std::function<HttpResponse(const HttpRequest&)>& handler) {
  Buffer data = tcp_.recv_app();
  if (data.empty()) return;
  std::string raw(data.begin(), data.end());
  HttpRequest req;
  if (!HttpRequest::parse(raw, req)) return;
  HttpResponse resp = handler(req);
  std::string s = resp.serialize();
  tcp_.send_app(Buffer(s.begin(), s.end()));
}

}  // namespace netsim
