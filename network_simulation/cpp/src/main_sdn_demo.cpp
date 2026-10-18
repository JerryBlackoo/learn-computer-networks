#include "netsim/host.hpp"
#include "netsim/sdn_controller.hpp"

#include <iostream>
#include <stdexcept>
#include <string>

using namespace netsim;

static void require(bool condition, const std::string& message) {
  if (!condition) throw std::runtime_error(message);
}

// Application policy lives entirely on the controller, never in SdnSwitch.
static std::optional<FlowEntry> policy(const PacketIn& packet) {
  std::cout << "[Packet-In] switch=" << packet.switch_id
            << " ingress=" << packet.ingress_port << "\n";
  FlowEntry entry;
  entry.match.ingress_port = packet.ingress_port;
  entry.match.ethertype = packet.frame.ethertype;
  if (packet.frame.ethertype == ETHERTYPE_ARP) {
    entry.id = 1 + packet.ingress_port;
    entry.priority = 10;
    entry.actions = {Forward{1 - packet.ingress_port}};
  } else if (packet.frame.ethertype == ETHERTYPE_IPV4) {
    Ipv4Packet ip;
    if (!Ipv4Packet::parse(packet.frame.payload, ip)) return std::nullopt;
    entry.match.src_ip = ip.src;
    entry.match.dst_ip = ip.dst;
    entry.match.ip_protocol = ip.protocol;
    if (ip.protocol == IPPROTO_TCP) {
      TcpSegment tcp;
      if (!TcpSegment::parse(ip.payload, ip.src, ip.dst, tcp)) return std::nullopt;
      Modify rewrite;
      if (packet.ingress_port == 0 && tcp.dst_port == 8080) {
        entry.id = 3;
        entry.match.dst_port = 8080;
        rewrite.dst_port = 80;
      } else if (packet.ingress_port == 1 && tcp.src_port == 80) {
        entry.id = 4;
        entry.match.src_port = 80;
        rewrite.src_port = 8080;
      } else {
        return std::nullopt;
      }
      entry.priority = 100;
      entry.actions = {rewrite, Forward{1 - packet.ingress_port}};
    } else if (ip.protocol == IPPROTO_UDP && packet.ingress_port == 0) {
      UdpDatagram udp;
      if (!UdpDatagram::parse(ip.payload, ip.src, ip.dst, udp) || udp.dst_port != 9999)
        return std::nullopt;
      entry.id = 5;
      entry.priority = 200;
      entry.match.dst_port = 9999;
      entry.actions = {Drop{}};
    } else {
      return std::nullopt;
    }
  } else {
    return std::nullopt;
  }
  std::cout << "[Flow-Mod] add id=" << entry.id << " priority=" << entry.priority << "\n";
  return entry;
}

int main() {
  try {
    std::cout << "SDN: client:40000 -> switch (8080 <-> 80) -> HTTP server:80\n";
    PhyLink client_to_switch, server_to_switch, switch_to_client, switch_to_server;
    SdnController controller(policy);
    SdnSwitch sw(1, 2);
    require(sw.set_port_tx(0, &switch_to_client), "client port attachment");
    require(sw.set_port_tx(1, &switch_to_server), "server port attachment");
    sw.set_packet_in_handler([&](SdnSwitch& source, const PacketIn& packet) {
      controller.on_packet_in(source, packet);
    });

    const auto client_ip = Ipv4Address::from_string("10.0.0.1");
    const auto server_ip = Ipv4Address::from_string("10.0.0.2");
    Host client("client"), server("server");
    client.add_iface({"eth0", MacAddress::from_string("02:00:00:00:00:01"), client_ip,
                      24, &client_to_switch, &switch_to_client});
    server.add_iface({"eth0", MacAddress::from_string("02:00:00:00:00:02"), server_ip,
                      24, &server_to_switch, &switch_to_server});
    const RouteEntry route{Ipv4Address::from_string("10.0.0.0"), 24, {}, "eth0"};
    client.add_route(route);
    server.add_route(route);
    client.bind_tcp_emit();
    server.bind_tcp_emit();
    client.tcp().local_ip = client_ip;
    client.tcp().local_port = 40000;
    server.tcp().local_ip = server_ip;
    server.tcp().listen(80);

    auto pump = [&]() {
      for (int i = 0; i < 40; ++i) {
        Buffer raw;
        while (client_to_switch.receive(raw)) sw.on_frame(0, raw);
        while (server_to_switch.receive(raw)) sw.on_frame(1, raw);
        client.poll();
        server.poll();
      }
    };
    client.tcp().connect(server_ip, 8080);
    pump();
    require(client.tcp().established() && server.tcp().established(), "TCP handshake failed");
    require(client.tcp().remote_port == 8080 && server.tcp().local_port == 80,
            "virtual/backend ports differ from policy");
    require(controller.packet_in_count() == 4 && controller.flow_mod_count() == 4,
            "expected two ARP rules and two TCP rewrite rules");

    HttpRequest request;
    request.method = "GET";
    request.path = "/sdn";
    request.host = "sdn.lab.local:8080";
    const auto text = request.serialize();
    const auto events_before_http = controller.packet_in_count();
    client.tcp().send_app(Buffer(text.begin(), text.end()));
    pump();
    server.serve_http_once([](const HttpRequest& received) {
      require(received.path == "/sdn", "unexpected HTTP path");
      HttpResponse response;
      response.body = "SDN flow-controlled HTTP OK";
      return response;
    });
    pump();
    const auto bytes = client.tcp().recv_app();
    HttpResponse response;
    require(HttpResponse::parse(std::string(bytes.begin(), bytes.end()), response) &&
                response.status == 200 && response.body == "SDN flow-controlled HTTP OK",
            "HTTP exchange failed after TCP port rewriting");
    require(controller.packet_in_count() == events_before_http, "HTTP should only hit installed flows");
    std::cout << "[Flow-Hit] HTTP 200, body=" << response.body << "; no new Packet-In\n";

    UdpDatagram blocked;
    blocked.src_port = 40001;
    blocked.dst_port = 9999;
    blocked.payload = {'b', 'l', 'o', 'c', 'k'};
    const auto forwarded_before_drop = sw.stats().forwarded;
    for (int i = 0; i < 2; ++i) {
      require(client.send_ip(server_ip, IPPROTO_UDP, blocked.serialize(client_ip, server_ip)),
              "blocked datagram submission failed");
      pump();
    }
    require(controller.packet_in_count() == 5 && controller.flow_mod_count() == 5,
            "drop rule should be installed only once");
    require(sw.flow_table().find(5) && sw.flow_table().find(5)->packet_count == 2,
            "both blocked packets must hit Drop");
    require(sw.stats().forwarded == forwarded_before_drop && sw.stats().dropped == 2,
            "blocked packets must never leave the switch");
    std::cout << "[Drop] UDP/9999 blocked twice; second packet hits installed rule\n";
    std::cout << "SDN e2e OK: Packet-In=" << controller.packet_in_count()
              << " Flow-Mod=" << controller.flow_mod_count()
              << " forwarded=" << sw.stats().forwarded << " dropped=" << sw.stats().dropped << "\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "FAIL: " << error.what() << "\n";
    return 1;
  }
}
