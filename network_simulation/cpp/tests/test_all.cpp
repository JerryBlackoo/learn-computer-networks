#include "netsim/arp.hpp"
#include "netsim/capacity.hpp"
#include "netsim/channel_util.hpp"
#include "netsim/checksum.hpp"
#include "netsim/crc.hpp"
#include "netsim/csma_cd.hpp"
#include "netsim/csma_ca.hpp"
#include "netsim/dhcp.hpp"
#include "netsim/icmp.hpp"
#include "netsim/token_ring.hpp"
#include "netsim/dns.hpp"
#include "netsim/email.hpp"
#include "netsim/ethernet.hpp"
#include "netsim/http.hpp"
#include "netsim/ipv4.hpp"
#include "netsim/mac_switch.hpp"
#include "netsim/phy.hpp"
#include "netsim/route.hpp"
#include "netsim/routing_algo.hpp"
#include "netsim/sliding_window.hpp"
#include "netsim/tcp.hpp"
#include "netsim/udp.hpp"

#include <cmath>
#include <iostream>
#include <string>
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

void test_capacity() {
  CHECK(std::abs(nyquist_bitrate(3000, 16) - 24000.0) < 1e-6);
  double shan = shannon_bitrate(3000, 30.0);
  CHECK(shan > 29800.0 && shan < 30000.0);
}

void test_phy() {
  PhyParams p;
  p.loss_prob = 0.5;
  p.seed = 42;
  PhyLink link(p);
  for (int i = 0; i < 50; ++i) link.send({'x'});
  Buffer out;
  int recvd = 0;
  while (link.receive(out)) ++recvd;
  CHECK(recvd > 10 && recvd < 40);
}

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
  raw[15] ^= 0x01;
  CHECK(!EthernetFrame::parse(raw, out));
}

void test_csma_cd() {
  CHECK(ethernet_min_frame_ok(64));
  CHECK(!ethernet_min_frame_ok(63));

  CsmaCdMac mac;
  CHECK(mac.carrier_idle());
  mac.set_busy_until(5);
  CHECK(!mac.carrier_idle());
  mac.advance(5);
  CHECK(mac.carrier_idle());

  auto ok = mac.try_transmit(5, false);
  CHECK(ok.success);
  CHECK(ok.collisions == 0);

  CsmaCdMac mac2;
  auto col = mac2.try_transmit(4, true);
  CHECK(col.success);
  CHECK(col.collisions == 1);
  CHECK(col.attempts == 2);
  CHECK(col.total_slots_waited >= 1);
  CHECK(mac2.backoff_slots(10) <= 1023);
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

void test_arp_poisoning_model() {
  ArpCache cache;
  Ipv4Address gw = Ipv4Address::from_string("192.168.5.254");
  cache.put(gw, MacAddress::from_string("00:11:22:33:44:55"));
  cache.put(gw, MacAddress::from_string("66:66:66:66:66:66"));
  auto m = cache.lookup(gw);
  CHECK(m && m->to_string() == "66:66:66:66:66:66");
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

void test_routing_algorithms() {
  DistanceVectorNode a("A"), b("B"), c("C");
  a.set_direct("B", 1);
  b.set_direct("A", 1);
  b.set_direct("C", 2);
  c.set_direct("B", 2);

  CHECK(a.cost_to("B") == 1);
  CHECK(a.cost_to("C") == kInfMetric);
  CHECK(a.on_update(b.advertisement()));
  CHECK(a.cost_to("C") == 3);
  CHECK(a.next_hop("C") == "B");

  std::vector<LsLink> links = {{"A", "B", 1}, {"B", "C", 2}, {"A", "D", 5}, {"C", "D", 1}};
  auto routes = dijkstra_routes("A", links);
  CHECK(routes["A"].cost == 0);
  CHECK(routes["B"].cost == 1);
  CHECK(routes["B"].next_hop == "B");
  CHECK(routes["C"].cost == 3);
  CHECK(routes["C"].next_hop == "B");
  CHECK(routes["D"].cost == 4);
  CHECK(routes["D"].next_hop == "B");
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
  GbnReceiver recv([&](const Buffer& d) { delivered.push_back(d); }, [&](uint32_t) {});
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
  CHECK(cc.ssthresh == 2);

  CongestionControl fr;
  fr.cwnd = 10;
  fr.ssthresh = 32;
  fr.on_dup_ack();
  fr.on_dup_ack();
  fr.on_dup_ack();
  CHECK(fr.dup_acks == 3);
  CHECK(fr.ssthresh == 5);
  CHECK(fr.cwnd == 8);
}

void test_dns_http() {
  DnsResolver dns;
  dns.add_record("www.example.com", Ipv4Address::from_string("1.2.3.4"));
  auto ip = dns.resolve("www.example.com");
  CHECK(ip && ip->to_string() == "1.2.3.4");

  HttpRequest req;
  req.method = "GET";
  req.path = "/xinxi";
  req.host = "www.example.com";
  HttpRequest back;
  CHECK(HttpRequest::parse(req.serialize(), back));
  CHECK(back.path == "/xinxi");
}

void test_channel_util() {
  // L/R = 1ms, prop=1ms => U = 1/(1+2)=1/3
  double u = stop_and_wait_utilization(1000, 1e6, 0.001);
  CHECK(std::abs(u - 1.0 / 3.0) < 1e-9);
  double ug = gbn_utilization(1000, 1e6, 0.001, 4);
  CHECK(std::abs(ug - 1.0) < 1e-9);  // W=4 > 1+2a=3
}

void test_mac_switch() {
  PhyLink h0_rx, h1_rx, h2_rx;
  MacLearningSwitch sw(3);
  sw.set_port_tx(0, &h0_rx);
  sw.set_port_tx(1, &h1_rx);
  sw.set_port_tx(2, &h2_rx);

  MacAddress m0 = MacAddress::from_string("02:00:00:00:00:01");
  MacAddress m1 = MacAddress::from_string("02:00:00:00:00:02");
  MacAddress m2 = MacAddress::from_string("02:00:00:00:00:03");

  EthernetFrame f;
  f.src = m0;
  f.dst = m1;
  f.ethertype = ETHERTYPE_IPV4;
  f.payload = {'x'};
  // Unknown dst -> flood to ports 1 and 2
  sw.on_frame(0, f.serialize());
  Buffer b;
  CHECK(h1_rx.receive(b));
  CHECK(h2_rx.receive(b));
  CHECK(!h0_rx.receive(b));
  CHECK(sw.lookup(m0) && *sw.lookup(m0) == 0);

  // Reply from m1 learns port 1; unicast back to m0 goes only to port 0
  f.src = m1;
  f.dst = m0;
  sw.on_frame(1, f.serialize());
  CHECK(h0_rx.receive(b));
  CHECK(!h2_rx.receive(b));
  CHECK(sw.lookup(m1) && *sw.lookup(m1) == 1);
}

void test_tcp_data_and_close() {
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
  CHECK(std::string(got.begin(), got.end()) == "ping");

  client.close();
  CHECK(client.state == TcpState::FinWait2 || client.state == TcpState::TimeWait);
  CHECK(server.state == TcpState::CloseWait);

  server.close();
  CHECK(server.state == TcpState::Closed || server.state == TcpState::LastAck);
  CHECK(client.state == TcpState::TimeWait || client.state == TcpState::FinWait2);
}

void test_csma_ca() {
  CsmaCaMac mac;
  CHECK(mac.current_cw() == 15);

  // Basic transmission (DATA -> SIFS -> ACK)
  auto res1 = mac.transmit(5, false, false);
  CHECK(res1.success);
  CHECK(res1.ack_received);
  CHECK(res1.attempts == 1);
  CHECK(res1.collisions == 0);
  CHECK(mac.current_cw() == 15);

  // Transmission with RTS/CTS 4-way handshake
  auto res_rts = mac.transmit(10, true, false);
  CHECK(res_rts.success);
  CHECK(res_rts.rts_cts_used);
  CHECK(res_rts.ack_received);

  // Transmission with collision on first attempt (CW doubles to 31, then resets on success)
  auto res_coll = mac.transmit(5, false, true);
  CHECK(res_coll.success);
  CHECK(res_coll.collisions == 1);
  CHECK(res_coll.attempts == 2);
  CHECK(mac.current_cw() == 15);
}

void test_token_ring() {
  TokenRing ring(4);
  CHECK(ring.station_count() == 4);
  CHECK(ring.total_frames_delivered() == 0);

  // Enqueue a frame from station 1 to station 3
  Buffer msg = {'T', 'O', 'K', 'E', 'N', '_', 'O', 'K'};
  ring.enqueue(1, 3, msg);

  int steps = ring.run_until_idle(50);
  CHECK(steps > 0);
  CHECK(ring.total_frames_delivered() == 1);

  // Verify station 3 received the exact message
  const auto& rx3 = ring.get_received(3);
  CHECK(rx3.size() == 1);
  CHECK(rx3[0] == msg);

  // Other stations received nothing
  CHECK(ring.get_received(0).empty());
  CHECK(ring.get_received(1).empty());
  CHECK(ring.get_received(2).empty());
}

void test_smtp_and_pop3() {
  // 1. SMTP Test
  SmtpServer smtp;
  CHECK(smtp.get_greeting().find("220") != std::string::npos);
  CHECK(smtp.process_line("HELO client.local\r\n").find("250") != std::string::npos);
  CHECK(smtp.process_line("MAIL FROM:<alice@lab.local>\r\n").find("250") != std::string::npos);
  CHECK(smtp.process_line("RCPT TO:<bob@lab.local>\r\n").find("250") != std::string::npos);
  CHECK(smtp.process_line("DATA\r\n").find("354") != std::string::npos);
  smtp.process_line("Subject: Hello Bob");
  smtp.process_line("This is a simulated email.");
  CHECK(smtp.process_line(".\r\n").find("250") != std::string::npos);
  CHECK(smtp.process_line("QUIT\r\n").find("221") != std::string::npos);

  CHECK(smtp.mail_count() == 1);
  const auto& spool = smtp.get_mail_spool();
  CHECK(spool[0].from == "<alice@lab.local>");
  CHECK(spool[0].to == "<bob@lab.local>");

  // 2. POP3 Test
  Pop3Server pop3("bob", "secret123");
  CHECK(pop3.get_greeting().find("+OK") != std::string::npos);

  // Deliver email to bob's mailbox
  pop3.add_email(spool[0]);
  CHECK(pop3.remaining_mail_count() == 1);

  // Auth
  CHECK(pop3.process_line("USER bob\r\n").find("+OK") != std::string::npos);
  CHECK(pop3.process_line("PASS wrong\r\n").find("-ERR") != std::string::npos);
  CHECK(pop3.process_line("PASS secret123\r\n").find("+OK") != std::string::npos);

  // STAT & LIST
  CHECK(pop3.process_line("STAT\r\n").find("+OK 1 ") != std::string::npos);
  CHECK(pop3.process_line("LIST\r\n").find("1 ") != std::string::npos);

  // RETR
  auto retr_resp = pop3.process_line("RETR 1\r\n");
  CHECK(retr_resp.find("+OK") != std::string::npos);
  CHECK(retr_resp.find("Hello Bob") != std::string::npos);

  // DELE & QUIT (Transaction -> Update)
  CHECK(pop3.process_line("DELE 1\r\n").find("+OK") != std::string::npos);
  CHECK(pop3.process_line("STAT\r\n").find("+OK 0 ") != std::string::npos);
  CHECK(pop3.process_line("QUIT\r\n").find("+OK") != std::string::npos);
  CHECK(pop3.remaining_mail_count() == 0);
}

void test_icmp() {
  // 1. Echo Request / Reply
  Buffer payload = {'P', 'I', 'N', 'G'};
  auto req = IcmpMessage::make_echo_request(0x1234, 1, payload);
  CHECK(req.type == ICMP_TYPE_ECHO_REQUEST);
  CHECK(req.code == 0);

  Buffer raw = req.serialize();
  CHECK(raw.size() == 8 + payload.size());

  IcmpMessage parsed_req;
  CHECK(IcmpMessage::parse(raw, parsed_req));
  CHECK(parsed_req.type == ICMP_TYPE_ECHO_REQUEST);
  CHECK(parsed_req.id == 0x1234);
  CHECK(parsed_req.sequence == 1);
  CHECK(parsed_req.payload == payload);

  // Corrupt a byte and verify checksum catches it
  raw[4] ^= 0xFF;
  IcmpMessage bad_req;
  CHECK(!IcmpMessage::parse(raw, bad_req));

  // 2. Error reporting messages: Time Exceeded & Dest Unreachable
  Buffer orig_ip = {'I', 'P', '_', 'H', 'E', 'A', 'D', 'E', 'R'};
  auto te = IcmpMessage::make_time_exceeded(orig_ip);
  CHECK(te.type == ICMP_TYPE_TIME_EXCEEDED);
  CHECK(te.code == 0);
  Buffer te_raw = te.serialize();
  IcmpMessage parsed_te;
  CHECK(IcmpMessage::parse(te_raw, parsed_te));
  CHECK(parsed_te.type == ICMP_TYPE_TIME_EXCEEDED);
  CHECK(parsed_te.payload == orig_ip);
}

void test_dhcp_handshake() {
  // Server on 192.168.1.1, pool 192.168.1.100 - 192.168.1.200
  Ipv4Address srv_ip = Ipv4Address::from_string("192.168.1.1");
  Ipv4Address pool_start = Ipv4Address::from_string("192.168.1.100");
  Ipv4Address mask = Ipv4Address::from_string("255.255.255.0");
  Ipv4Address gw = Ipv4Address::from_string("192.168.1.1");
  Ipv4Address dns = Ipv4Address::from_string("8.8.8.8");

  DhcpServer server(srv_ip, pool_start, 50, mask, gw, dns);

  MacAddress client_mac = MacAddress::from_string("00:11:22:33:44:55");
  DhcpClient client(client_mac);
  CHECK(client.state() == DhcpClient::State::INIT);

  // 1. Client creates DHCPDISCOVER (broadcast)
  DhcpPacket discover = client.create_discover(0xDEADBEEF);
  CHECK(client.state() == DhcpClient::State::SELECTING);
  Buffer disc_raw = discover.serialize();
  DhcpPacket disc_parsed;
  CHECK(DhcpPacket::parse(disc_raw, disc_parsed));
  CHECK(disc_parsed.type == DhcpMessageType::DISCOVER);

  // 2. Server receives DISCOVER and creates DHCPOFFER
  auto offer_opt = server.handle_packet(disc_parsed);
  CHECK(offer_opt.has_value());
  CHECK(offer_opt->type == DhcpMessageType::OFFER);
  CHECK(offer_opt->yiaddr == pool_start); // Allocated 192.168.1.100

  // 3. Client receives OFFER and creates DHCPREQUEST
  auto req_opt = client.handle_offer(*offer_opt);
  CHECK(req_opt.has_value());
  CHECK(client.state() == DhcpClient::State::REQUESTING);
  CHECK(req_opt->type == DhcpMessageType::REQUEST);
  CHECK(req_opt->yiaddr == pool_start);

  // 4. Server receives REQUEST and creates DHCPACK
  auto ack_opt = server.handle_packet(*req_opt);
  CHECK(ack_opt.has_value());
  CHECK(ack_opt->type == DhcpMessageType::ACK);

  // 5. Client receives ACK -> State becomes BOUND
  CHECK(client.handle_ack(*ack_opt));
  CHECK(client.state() == DhcpClient::State::BOUND);
  CHECK(client.ip() == pool_start);
  CHECK(client.mask() == mask);
  CHECK(client.gateway() == gw);
  CHECK(client.dns() == dns);
  CHECK(server.active_lease_count() == 1);
}

int main() {
  test_capacity();
  test_phy();
  test_crc_and_ethernet();
  test_csma_cd();
  test_csma_ca();
  test_token_ring();
  test_smtp_and_pop3();
  test_icmp();
  test_dhcp_handshake();
  test_checksum_ip_udp();
  test_arp_poisoning_model();
  test_lpm();
  test_routing_algorithms();
  test_gbn();
  test_congestion();
  test_dns_http();
  test_channel_util();
  test_mac_switch();
  test_tcp_data_and_close();

  if (g_failed) {
    std::cerr << g_failed << " checks failed\n";
    return 1;
  }
  std::cout << "all unit checks passed\n";
  return 0;
}
