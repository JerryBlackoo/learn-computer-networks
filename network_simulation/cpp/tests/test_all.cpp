#include "netsim/arp.hpp"
#include "netsim/checksum.hpp"
#include "netsim/crc.hpp"
#include "netsim/dns.hpp"
#include "netsim/ethernet.hpp"
#include "netsim/http.hpp"
#include "netsim/ipv4.hpp"
#include "netsim/route.hpp"
#include "netsim/sliding_window.hpp"
#include "netsim/tcp.hpp"
#include "netsim/udp.hpp"

#include <cassert>
#include <iostream>
#include <vector>

using namespace netsim;

static int g_failed = 0;

#define CHECK(cond)                                                                              \
  do {                                                                                           \
    if (!(cond)) {                                                                               \
      std::cerr << "FAIL " << __FILE__ << ":" << __LINE__ << " " << #cond << "\n";             \
      ++g_failed;                                                                                \
    }                                                                                            \
  } while (0)

void test_crc_and_ethernet() {
  EthernetFrame f;
  f.dst = MacAddress::from_string("aa:bb:cc:dd:ee:ff");
  f.src = MacAddress::from_string("11:22:33:44:55:66");
  f.ethertype = ETHERTYPE_IPV4;
  f.payload = {'h', 'i'};
  Buffer raw = f.serialize();
  EthernetFrame out;
  CHECK(EthernetFrame::parse(raw, out));
  CHECK(out.payload.size() == 2);
  raw[15] ^= 0x01;  // corrupt payload/header byte (not past end)
  CHECK(!EthernetFrame::parse(raw, out));
}

void test_checksum_ip_udp() {
  Ipv4Address a = Ipv4Address::from_string("10.0.0.1");
  Ipv4Address b = Ipv4Address::from_string("10.0.0.2");
  UdpDatagram u;
  u.src_port = 12345;
  u.dst_port = 53;
  u.payload = {'Q', 'x', 0};
  Buffer enc = u.serialize(a, b);
  UdpDatagram back;
  CHECK(UdpDatagram::parse(enc, a, b, back));
  CHECK(back.dst_port == 53);

  Ipv4Packet ip;
  ip.src = a;
  ip.dst = b;
  ip.protocol = IPPROTO_UDP;
  ip.payload = enc;
  Buffer ipraw = ip.serialize();
  Ipv4Packet ip2;
  CHECK(Ipv4Packet::parse(ipraw, ip2));
  CHECK(ip2.dst == b);
}

void test_lpm() {
  RouteTable t;
  t.add({Ipv4Address::from_string("10.0.0.0"), 8, Ipv4Address::from_string("1.1.1.1"), "eth0"});
  t.add({Ipv4Address::from_string("10.1.2.0"), 24, Ipv4Address::from_string("2.2.2.2"), "eth1"});
  t.add({Ipv4Address::from_string("0.0.0.0"), 0, Ipv4Address::from_string("9.9.9.9"), "eth0"});
  auto r = t.longest_prefix_match(Ipv4Address::from_string("10.1.2.5"));
  CHECK(r && r->prefix_len == 24);
  r = t.longest_prefix_match(Ipv4Address::from_string("10.9.9.9"));
  CHECK(r && r->prefix_len == 8);
  r = t.longest_prefix_match(Ipv4Address::from_string("8.8.8.8"));
  CHECK(r && r->prefix_len == 0);
}

void test_gbn() {
  std::vector<std::pair<uint32_t, Buffer>> sent;
  GbnSender sender(2, [&](uint32_t seq, const Buffer& d) { sent.push_back({seq, d}); });
  sender.enqueue({'a'});
  sender.enqueue({'b'});
  sender.enqueue({'c'});
  CHECK(sent.size() == 2);
  sender.on_ack(0);
  CHECK(sent.size() == 3);
  std::vector<Buffer> delivered;
  GbnReceiver recv(
      [&](const Buffer& d) { delivered.push_back(d); },
      [&](uint32_t) {});
  recv.on_frame(0, {'a'});
  recv.on_frame(1, {'b'});
  CHECK(delivered.size() == 2);
}

void test_congestion() {
  CongestionControl cc;
  cc.cwnd = 1;
  cc.ssthresh = 8;
  for (int i = 0; i < 4; ++i) cc.on_ack_new_data();
  CHECK(cc.cwnd == 5);
  cc.on_timeout();
  CHECK(cc.cwnd == 1);
  CHECK(cc.ssthresh == 2);  // max(5/2, 2)
}

void test_dns_http() {
  DnsResolver dns;
  dns.add_record("www.example.com", Ipv4Address::from_string("1.2.3.4"));
  auto ip = dns.resolve("www.example.com");
  CHECK(ip && ip->to_string() == "1.2.3.4");
  auto q = DnsResolver::encode_query("www.example.com");
  std::string name;
  CHECK(DnsResolver::decode_query(q, name));
  CHECK(name == "www.example.com");

  HttpRequest req;
  req.method = "GET";
  req.path = "/xinxi";
  req.host = "www.example.com";
  auto s = req.serialize();
  HttpRequest back;
  CHECK(HttpRequest::parse(s, back));
  CHECK(back.path == "/xinxi");
}

void test_tcp_handshake_memory() {
  // Direct PCB wiring without IP
  TcpPcb client, server;
  client.iss = 1000;
  server.iss = 5000;
  server.listen(80);
  client.local_port = 40000;
  client.remote_port = 80;

  std::vector<TcpSegment> wire;
  client.emit = [&](const TcpSegment& s) { wire.push_back(s); };
  server.emit = [&](const TcpSegment& s) { wire.push_back(s); };

  client.connect(Ipv4Address::from_string("10.0.0.2"), 80);
  CHECK(wire.size() == 1 && (wire[0].flags & TcpSegment::SYN));
  server.on_segment(wire[0]);
  wire.clear();
  // server emitted SYN+ACK via emit during on_segment
  // Need to capture: rebind to push after listen path
}

int main() {
  test_crc_and_ethernet();
  test_checksum_ip_udp();
  test_lpm();
  test_gbn();
  test_congestion();
  test_dns_http();

  // TCP handshake with bidirectional emit
  {
    TcpPcb client, server;
    client.iss = 1000;
    server.iss = 5000;
    server.listen(80);
    client.local_port = 40000;

    client.emit = [&](const TcpSegment& s) { server.on_segment(s); };
    server.emit = [&](const TcpSegment& s) { client.on_segment(s); };

    client.connect(Ipv4Address::from_string("10.0.0.2"), 80);
    CHECK(client.established());
    CHECK(server.established());

    client.send_app(Buffer{'p', 'i', 'n', 'g'});
    auto got = server.recv_app();
    CHECK(got.size() == 4);
    CHECK(std::string(got.begin(), got.end()) == "ping");

    // congestion path exercised via ACKs
    CHECK(client.cc.cwnd >= 1);
  }

  if (g_failed) {
    std::cerr << g_failed << " checks failed\n";
    return 1;
  }
  std::cout << "all unit checks passed\n";
  return 0;
}
