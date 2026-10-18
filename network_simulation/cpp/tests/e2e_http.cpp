#include "netsim/host.hpp"

#include <iostream>
#include <string>

using namespace netsim;

static void pump(Host& a, Host& b, int rounds = 30) {
  for (int i = 0; i < rounds; ++i) {
    a.poll();
    b.poll();
  }
}

int main() {
  // client.tx -> server.rx ; server.tx -> client.rx
  PhyLink c2s;
  PhyLink s2c;

  Host client("client");
  Host server("server");

  client.add_iface(Iface{"eth0", MacAddress::from_string("02:00:00:00:00:01"),
                         Ipv4Address::from_string("10.0.0.1"), 24, &c2s, &s2c});
  server.add_iface(Iface{"eth0", MacAddress::from_string("02:00:00:00:00:02"),
                         Ipv4Address::from_string("10.0.0.2"), 24, &s2c, &c2s});

  client.add_route({Ipv4Address::from_string("10.0.0.0"), 24, Ipv4Address{}, "eth0"});
  server.add_route({Ipv4Address::from_string("10.0.0.0"), 24, Ipv4Address{}, "eth0"});

  server.dns().add_record("www.lab.local", Ipv4Address::from_string("10.0.0.2"));

  client.bind_tcp_emit();
  server.bind_tcp_emit();
  client.tcp().local_ip = Ipv4Address::from_string("10.0.0.1");
  server.tcp().local_ip = Ipv4Address::from_string("10.0.0.2");

  // --- DNS over UDP path (triggers ARP + server DNS reply) ---
  {
    auto q = DnsResolver::encode_query("www.lab.local");
    UdpDatagram udp;
    udp.src_port = 53000;
    udp.dst_port = 53;
    udp.payload = q;
    auto enc = udp.serialize(Ipv4Address::from_string("10.0.0.1"),
                             Ipv4Address::from_string("10.0.0.2"));
    client.send_ip(Ipv4Address::from_string("10.0.0.2"), IPPROTO_UDP, enc);
    pump(client, server, 15);

    auto mac = client.arp().lookup(Ipv4Address::from_string("10.0.0.2"));
    if (!mac) {
      std::cerr << "FAIL: ARP not learned after DNS query\n";
      return 1;
    }
  }

  // --- TCP + HTTP ---
  server.tcp().listen(80);
  client.tcp().local_port = 40000;
  client.tcp().connect(Ipv4Address::from_string("10.0.0.2"), 80);
  pump(client, server, 40);

  if (!client.tcp().established() || !server.tcp().established()) {
    std::cerr << "FAIL: TCP not established\n";
    return 1;
  }

  HttpRequest req;
  req.method = "GET";
  req.path = "/xinxi";
  req.host = "www.lab.local";
  std::string req_s = req.serialize();
  client.tcp().send_app(Buffer(req_s.begin(), req_s.end()));
  pump(client, server, 20);

  server.serve_http_once([](const HttpRequest& r) {
    HttpResponse resp;
    resp.status = 200;
    resp.reason = "OK";
    resp.content_type = "application/json";
    resp.body = std::string("{\"path\":\"") + r.path + "\",\"ok\":true}";
    return resp;
  });
  pump(client, server, 20);

  Buffer resp_bytes = client.tcp().recv_app();
  std::string resp_s(resp_bytes.begin(), resp_bytes.end());
  HttpResponse resp;
  if (!HttpResponse::parse(resp_s, resp) || resp.status != 200 ||
      resp.body.find("\"ok\":true") == std::string::npos) {
    std::cerr << "FAIL: HTTP response bad: [" << resp_s << "]\n";
    return 1;
  }

  std::cout << "e2e OK: ARP + DNS query path + TCP handshake + HTTP GET /xinxi\n";
  std::cout << "body=" << resp.body << "\n";
  return 0;
}
