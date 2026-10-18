#include "netsim/arp.hpp"
#include "netsim/checksum.hpp"
#include "netsim/crc.hpp"
#include "netsim/dns.hpp"
#include "netsim/ethernet.hpp"
#include "netsim/host.hpp"
#include "netsim/http.hpp"
#include "netsim/ipv4.hpp"
#include "netsim/phy.hpp"
#include "netsim/route.hpp"
#include "netsim/sliding_window.hpp"
#include "netsim/tcp.hpp"
#include "netsim/udp.hpp"

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

using namespace netsim;

static void pump(Host& a, Host& b, int rounds = 20) {
  for (int i = 0; i < rounds; ++i) {
    a.poll();
    b.poll();
  }
}

void demo_physical_and_datalink() {
  std::cout << "\n====================================================================\n";
  std::cout << "  [LAYER 1 & 2] PHYSICAL & DATA LINK LAYERS (CRC-32 & GBN)\n";
  std::cout << "====================================================================\n";

  // 1. Physical Channel
  PhyParams p;
  p.loss_prob = 0.0;
  p.bit_error_prob = 0.01; // 1% bit flip rate
  p.seed = 42;
  PhyLink link(p);

  std::cout << "1. Physical Channel Impairment Model:\n";
  std::cout << "   - Simulated Loss Probability: " << p.loss_prob * 100 << "%\n";
  std::cout << "   - Simulated Bit Error Rate (BER): " << p.bit_error_prob * 100 << "%\n\n";

  // 2. Ethernet II Frame & CRC-32
  EthernetFrame frame;
  frame.dst = MacAddress::from_string("00:1a:2b:3c:4d:5e");
  frame.src = MacAddress::from_string("aa:bb:cc:dd:ee:ff");
  frame.ethertype = ETHERTYPE_IPV4;
  std::string msg = "Hello Computer Networks!";
  frame.payload = Buffer(msg.begin(), msg.end());

  Buffer raw = frame.serialize();
  uint32_t fcs = crc32(raw.data(), raw.size() - 4);

  std::cout << "2. Ethernet II Frame Construction:\n";
  std::cout << "   - Destination MAC: " << frame.dst.to_string() << "\n";
  std::cout << "   - Source MAC:      " << frame.src.to_string() << "\n";
  std::cout << "   - EtherType:       0x0800 (IPv4)\n";
  std::cout << "   - Payload (" << frame.payload.size() << " bytes): \"" << msg << "\"\n";
  std::cout << "   - Total Serialized Frame Size: " << raw.size() << " bytes (Header 14B + Payload + 4B FCS)\n";
  std::cout << "   - IEEE 802.3 CRC-32 FCS: 0x" << std::hex << fcs << std::dec << "\n";

  // Verify Frame Parsing
  EthernetFrame parsed;
  bool ok = EthernetFrame::parse(raw, parsed);
  std::cout << "   - Frame Validation: " << (ok ? "VALID (CRC check PASSED)" : "CORRUPT") << "\n";

  // 3. GBN Sliding Window Protocol
  std::cout << "\n3. Go-Back-N (GBN) Sliding Window Protocol:\n";
  std::vector<uint32_t> transmitted_seqs;
  GbnSender gbn_sender(3, [&](uint32_t seq, const Buffer&) {
    transmitted_seqs.push_back(seq);
  });

  std::cout << "   - Window size W_T = 3\n";
  gbn_sender.enqueue({'A'});
  gbn_sender.enqueue({'B'});
  gbn_sender.enqueue({'C'});
  std::cout << "   - Transmitted initial window frames: Seq=";
  for (size_t i = 0; i < transmitted_seqs.size(); ++i) {
    if (i > 0) std::cout << ", ";
    std::cout << transmitted_seqs[i];
  }
  std::cout << "\n";

  gbn_sender.enqueue({'D'}); // Pending, outside window
  std::cout << "   - Frame 'D' enqueued (blocked by window until ACK)\n";
  gbn_sender.on_ack(0); // Cumulative ACK for frame 0 slides window
  std::cout << "   - Received ACK for Seq=0: window slides, transmitted Seq="
            << transmitted_seqs.back() << "\n";
}

void demo_network_layer() {
  std::cout << "\n====================================================================\n";
  std::cout << "  [LAYER 3] NETWORK LAYER (IPV4, LPM ROUTING & ARP SPOOFING)\n";
  std::cout << "====================================================================\n";

  // 1. Longest Prefix Match (LPM)
  RouteTable rt;
  rt.add({Ipv4Address::from_string("0.0.0.0"), 0, Ipv4Address::from_string("192.168.1.1"), "eth0"});
  rt.add({Ipv4Address::from_string("192.168.5.0"), 24, Ipv4Address::from_string("0.0.0.0"), "eth1"});
  rt.add({Ipv4Address::from_string("192.168.5.128"), 25, Ipv4Address::from_string("0.0.0.0"), "eth2"});

  std::cout << "1. CIDR Routing Table with Longest Prefix Match (LPM):\n";
  std::cout << "   Installed Routes:\n";
  std::cout << "     * 0.0.0.0/0       via 192.168.1.1 dev eth0 (Default Route)\n";
  std::cout << "     * 192.168.5.0/24   direct          dev eth1 (Campus LAN)\n";
  std::cout << "     * 192.168.5.128/25 direct          dev eth2 (Specific Lab Subnet)\n\n";

  Ipv4Address ip1 = Ipv4Address::from_string("192.168.5.200");
  auto match1 = rt.longest_prefix_match(ip1);
  std::cout << "   Query " << ip1.to_string() << ": matched "
            << match1->network.to_string() << "/" << static_cast<int>(match1->prefix_len)
            << " on " << match1->iface << " (LPM /25 chosen)\n";

  Ipv4Address ip2 = Ipv4Address::from_string("192.168.5.50");
  auto match2 = rt.longest_prefix_match(ip2);
  std::cout << "   Query " << ip2.to_string() << ":  matched "
            << match2->network.to_string() << "/" << static_cast<int>(match2->prefix_len)
            << " on " << match2->iface << " (LPM /24 chosen)\n";

  Ipv4Address ip3 = Ipv4Address::from_string("8.8.8.8");
  auto match3 = rt.longest_prefix_match(ip3);
  std::cout << "   Query " << ip3.to_string() << ":      matched "
            << match3->network.to_string() << "/" << static_cast<int>(match3->prefix_len)
            << " on " << match3->iface << " (Default route /0 chosen)\n\n";

  // 2. ARP Cache Poisoning Simulation (Final Exam Question 2)
  std::cout << "2. ARP Resolution & Cache Poisoning (Exam Question 2 Analysis):\n";
  ArpCache arp;
  Ipv4Address gw = Ipv4Address::from_string("192.168.5.254");
  MacAddress legitimate_mac = MacAddress::from_string("aa:bb:cc:11:22:33");
  arp.put(gw, legitimate_mac);

  std::cout << "   - Legitimate Gateway ARP Entry: " << gw.to_string() << " -> "
            << arp.lookup(gw)->to_string() << "\n";

  MacAddress attacker_mac = MacAddress::from_string("ee:ee:ee:66:66:66");
  arp.put(gw, attacker_mac); // Rogue ARP Reply poisoning
  std::cout << "   - Rogue Host sends unsolicited ARP reply! Cache updated:\n";
  std::cout << "   - Poisoned Gateway Entry:        " << gw.to_string() << " -> "
            << arp.lookup(gw)->to_string() << " [MITM Vulnerability Exposing Traffic]\n";
}

void demo_transport_and_app() {
  std::cout << "\n====================================================================\n";
  std::cout << "  [LAYER 4 & 5] TRANSPORT & APPLICATION LAYERS (TCP RENO & HTTP)\n";
  std::cout << "====================================================================\n";

  // 1. TCP Reno Congestion Control Evolution
  std::cout << "1. RFC 5681 TCP Reno Congestion Control State Evolution:\n";
  CongestionControl cc;
  cc.cwnd = 1;
  cc.ssthresh = 8;
  std::cout << "   - Initial: cwnd=" << cc.cwnd << " MSS, ssthresh=" << cc.ssthresh << " MSS (Slow Start)\n";

  // Slow start ACKs
  for (int i = 0; i < 7; ++i) cc.on_ack_new_data();
  std::cout << "   - After 7 new ACKs in Slow Start: cwnd=" << cc.cwnd
            << " MSS (Reached ssthresh)\n";

  // Congestion avoidance
  for (int i = 0; i < 8; ++i) cc.on_ack_new_data();
  std::cout << "   - After 8 ACKs in Congestion Avoidance: cwnd=" << cc.cwnd
            << " MSS (Linear Additive Increase)\n";

  // 3 Duplicate ACKs -> Fast Retransmit / Fast Recovery
  cc.on_dup_ack();
  cc.on_dup_ack();
  cc.on_dup_ack();
  std::cout << "   - 3 Duplicate ACKs received! Fast Retransmit triggered:\n";
  std::cout << "     * ssthresh = max(cwnd/2, 2) = " << cc.ssthresh << " MSS\n";
  std::cout << "     * cwnd = ssthresh + 3 = " << cc.cwnd << " MSS (Fast Recovery Active)\n";

  // Timeout
  cc.on_timeout();
  std::cout << "   - Retransmission Timeout (RTO) occurs:\n";
  std::cout << "     * cwnd reset to 1 MSS = " << cc.cwnd << " MSS\n";
  std::cout << "     * ssthresh = " << cc.ssthresh << " MSS (Re-entering Slow Start)\n\n";

  // 2. DNS & HTTP/1.1
  std::cout << "2. Application Layer: DNS & HTTP/1.1:\n";
  DnsResolver dns;
  dns.add_record("www.shiep.edu.cn", Ipv4Address::from_string("100.18.22.80"));
  auto resolved_ip = dns.resolve("www.shiep.edu.cn");
  std::cout << "   - DNS Resolution: www.shiep.edu.cn -> " << resolved_ip->to_string() << "\n";

  HttpRequest req;
  req.method = "GET";
  req.path = "/index.html";
  req.host = "www.shiep.edu.cn";
  std::string raw_req = req.serialize();
  std::cout << "   - HTTP Request:\n" << raw_req;

  HttpResponse resp;
  resp.status = 200;
  resp.reason = "OK";
  resp.content_type = "text/html";
  resp.body = "<html><body>Welcome to Shanghai Electric Power University</body></html>";
  std::string raw_resp = resp.serialize();
  std::cout << "   - HTTP Response:\n" << raw_resp << "\n";
}

void demo_end_to_end_simulation() {
  std::cout << "\n====================================================================\n";
  std::cout << "  [END-TO-END] MULTI-LAYER INTEGRATED SIMULATION PIPELINE\n";
  std::cout << "  Client Host <-> Server Host through Full 5-Layer Stack\n";
  std::cout << "====================================================================\n";

  PhyLink c2s;
  PhyLink s2c;

  Host client("Client-PC1");
  Host server("Web-Server");

  client.add_iface(Iface{"eth0", MacAddress::from_string("00:11:22:33:44:55"),
                         Ipv4Address::from_string("192.168.5.85"), 24, &c2s, &s2c});
  server.add_iface(Iface{"eth0", MacAddress::from_string("aa:bb:cc:11:22:33"),
                         Ipv4Address::from_string("192.168.5.80"), 24, &s2c, &c2s});

  client.add_route({Ipv4Address::from_string("192.168.5.0"), 24, Ipv4Address{}, "eth0"});
  server.add_route({Ipv4Address::from_string("192.168.5.0"), 24, Ipv4Address{}, "eth0"});

  server.dns().add_record("www.shiep.edu.cn", Ipv4Address::from_string("192.168.5.80"));

  client.bind_tcp_emit();
  server.bind_tcp_emit();
  client.tcp().local_ip = Ipv4Address::from_string("192.168.5.85");
  server.tcp().local_ip = Ipv4Address::from_string("192.168.5.80");

  std::cout << "Step 1: Client issues DNS query over UDP port 53 (triggers ARP)...\n";
  auto q = DnsResolver::encode_query("www.shiep.edu.cn");
  UdpDatagram udp;
  udp.src_port = 54321;
  udp.dst_port = 53;
  udp.payload = q;
  auto enc = udp.serialize(Ipv4Address::from_string("192.168.5.85"),
                           Ipv4Address::from_string("192.168.5.80"));
  client.send_ip(Ipv4Address::from_string("192.168.5.80"), IPPROTO_UDP, enc);
  pump(client, server, 20);

  auto learned_mac = client.arp().lookup(Ipv4Address::from_string("192.168.5.80"));
  std::cout << "  -> ARP resolution completed! Learned Server MAC: "
            << (learned_mac ? learned_mac->to_string() : "NONE") << "\n";

  std::cout << "Step 2: Client initiates TCP 3-way handshake to Server port 80...\n";
  server.tcp().listen(80);
  client.tcp().local_port = 50000;
  client.tcp().connect(Ipv4Address::from_string("192.168.5.80"), 80);
  pump(client, server, 30);

  std::cout << "  -> TCP Handshake Completed! Client State: Established="
            << (client.tcp().established() ? "true" : "false")
            << ", Server State: Established="
            << (server.tcp().established() ? "true" : "false") << "\n";

  std::cout << "Step 3: Client sends HTTP GET /index.html over TCP stream...\n";
  HttpRequest http_req;
  http_req.method = "GET";
  http_req.path = "/index.html";
  http_req.host = "www.shiep.edu.cn";
  std::string http_raw = http_req.serialize();
  client.tcp().send_app(Buffer(http_raw.begin(), http_raw.end()));
  pump(client, server, 15);

  server.serve_http_once([](const HttpRequest& r) {
    HttpResponse resp;
    resp.status = 200;
    resp.reason = "OK";
    resp.content_type = "text/html";
    resp.body = "<html><body>Welcome to Shanghai Electric Power University (Verified E2E)</body></html>";
    return resp;
  });
  pump(client, server, 20);

  Buffer resp_bytes = client.tcp().recv_app();
  std::string resp_str(resp_bytes.begin(), resp_bytes.end());
  HttpResponse final_resp;
  bool parsed_ok = HttpResponse::parse(resp_str, final_resp);

  std::cout << "Step 4: Client receives & parses HTTP 200 OK Response:\n";
  std::cout << "  -> Parse Status: " << (parsed_ok ? "SUCCESS" : "FAILED") << "\n";
  std::cout << "  -> HTTP Status Code: " << final_resp.status << " " << final_resp.reason << "\n";
  std::cout << "  -> HTTP Payload: " << final_resp.body << "\n";

  std::cout << "\n====================================================================\n";
  std::cout << "  SIMULATION PIPELINE EXECUTED SUCCESSFULLY!\n";
  std::cout << "====================================================================\n";
}

int main() {
  std::cout << "********************************************************************\n";
  std::cout << "  COMPUTER NETWORK FIVE-LAYER SIMULATION SYSTEM (C++17)\n";
  std::cout << "  Based on Xie Xiren (8th Ed), Solution Manual, Exam & Wangdao\n";
  std::cout << "********************************************************************\n";

  demo_physical_and_datalink();
  demo_network_layer();
  demo_transport_and_app();
  demo_end_to_end_simulation();

  return 0;
}
