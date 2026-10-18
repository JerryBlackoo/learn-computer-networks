#include "netsim/checksum.hpp"
#include "netsim/ipv4.hpp"
#include "netsim/sdn_controller.hpp"
#include "netsim/tcp.hpp"
#include "netsim/udp.hpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>

using namespace netsim;

// Deliberately not assert(): checks must also run in Release/NDEBUG builds.
#define CHECK(cond)                                                                            \
  do {                                                                                         \
    if (!(cond))                                                                               \
      throw std::runtime_error(std::string(__FILE__) + ":" + std::to_string(__LINE__) +          \
                               " " + #cond);                                                   \
  } while (0)

static Ipv4Address ip(const std::string& value) { return Ipv4Address::from_string(value); }
static MacAddress mac(const std::string& value) { return MacAddress::from_string(value); }

static EthernetFrame make_frame(uint8_t protocol = IPPROTO_UDP) {
  Ipv4Packet packet;
  packet.src = ip("10.0.0.1");
  packet.dst = ip("10.0.0.2");
  packet.protocol = protocol;
  if (protocol == IPPROTO_UDP) {
    UdpDatagram udp;
    udp.src_port = 12345;
    udp.dst_port = 53;
    udp.payload = {'s', 'd', 'n'};
    packet.payload = udp.serialize(packet.src, packet.dst);
  } else if (protocol == IPPROTO_TCP) {
    TcpSegment tcp;
    tcp.src_port = 40000;
    tcp.dst_port = 80;
    tcp.flags = TcpSegment::ACK;
    tcp.seq = 100;
    tcp.ack = 200;
    tcp.payload = {'G', 'E', 'T'};
    packet.payload = tcp.serialize(packet.src, packet.dst);
  } else {
    packet.payload = {8, 0, 0, 0};
  }
  EthernetFrame frame;
  frame.src = mac("02:00:00:00:00:01");
  frame.dst = mac("02:00:00:00:00:02");
  frame.payload = packet.serialize();
  return frame;
}

static FlowEntry rule(uint64_t id, uint16_t priority, std::vector<FlowAction> actions) {
  FlowEntry entry;
  entry.id = id;
  entry.priority = priority;
  entry.actions = std::move(actions);
  return entry;
}

static void repair_ip_checksum(Buffer& packet) {
  write_u16_be(packet.data() + 10, 0);
  write_u16_be(packet.data() + 10, internet_checksum(packet.data(), (packet[0] & 15) * 4));
}

static void test_matches() {
  const auto frame = make_frame();
  FlowMatch any;
  CHECK(any.matches(0, frame));
  auto exact = any;
  exact.ingress_port = 0;
  exact.src_mac = frame.src;
  exact.dst_mac = frame.dst;
  exact.ethertype = ETHERTYPE_IPV4;
  exact.src_ip = ip("10.0.0.1");
  exact.dst_ip = ip("10.0.0.2");
  exact.ip_protocol = IPPROTO_UDP;
  exact.src_port = 12345;
  exact.dst_port = 53;
  CHECK(exact.matches(0, frame));
  CHECK(!exact.matches(1, frame));

  // Every field is independently significant; wildcards are field-wise ANDs.
  auto mismatch = exact;
  mismatch.src_mac = MacAddress{};
  CHECK(!mismatch.matches(0, frame));
  mismatch = exact;
  mismatch.dst_mac = MacAddress{};
  CHECK(!mismatch.matches(0, frame));
  mismatch = exact;
  mismatch.ethertype = ETHERTYPE_ARP;
  CHECK(!mismatch.matches(0, frame));
  mismatch = exact;
  mismatch.src_ip = ip("10.0.0.3");
  CHECK(!mismatch.matches(0, frame));
  mismatch = exact;
  mismatch.dst_ip = ip("10.0.0.3");
  CHECK(!mismatch.matches(0, frame));
  mismatch = exact;
  mismatch.ip_protocol = IPPROTO_TCP;
  CHECK(!mismatch.matches(0, frame));
  mismatch = exact;
  mismatch.src_port = 0;
  CHECK(!mismatch.matches(0, frame));
  mismatch = exact;
  mismatch.dst_port = 0;
  CHECK(!mismatch.matches(0, frame));
  FlowMatch l4;
  l4.dst_port = 53;
  CHECK(l4.matches(7, frame));
  CHECK(!l4.matches(7, make_frame(IPPROTO_TCP)));
  l4.dst_port = 80;
  CHECK(l4.matches(7, make_frame(IPPROTO_TCP)));
  CHECK(!l4.matches(7, make_frame(IPPROTO_ICMP)));

  auto arp = frame;
  arp.ethertype = ETHERTYPE_ARP;
  CHECK(any.matches(0, arp));
  CHECK(!exact.matches(0, arp));
  CHECK(!l4.matches(0, arp));
  FlowMatch l3;
  l3.src_ip = Ipv4Address{};  // 0.0.0.0 is not a wildcard.
  CHECK(!l3.matches(0, frame));
  auto zero = frame;
  Ipv4Address{}.write_bytes(zero.payload.data() + 12);
  repair_ip_checksum(zero.payload);
  CHECK(l3.matches(0, zero));
  FlowMatch zero_port;
  zero_port.src_port = 0;
  write_u16_be(zero.payload.data() + 20, 0);
  CHECK(zero_port.matches(0, zero));
}

static void test_malformed_and_fragments() {
  const auto frame = make_frame();
  FlowMatch l3;
  l3.ip_protocol = IPPROTO_UDP;
  FlowMatch l4;
  l4.dst_port = 53;
  for (std::size_t n = 0; n < frame.payload.size(); ++n) {
    auto short_frame = frame;
    short_frame.payload.resize(n);
    CHECK(!l3.matches(0, short_frame));
    CHECK(!l4.matches(0, short_frame));
  }
  auto bad = frame;
  write_u16_be(bad.payload.data() + 2, 19);  // total_length < IHL
  repair_ip_checksum(bad.payload);
  CHECK(!l3.matches(0, bad));
  bad = frame;
  bad.payload[0] = 0x44;
  CHECK(!l3.matches(0, bad));
  bad = frame;
  bad.payload[8] ^= 1;  // Invalid IP checksum.
  CHECK(!l3.matches(0, bad));
  bad = frame;
  write_u16_be(bad.payload.data() + 24, 7);  // Invalid UDP length.
  CHECK(l3.matches(0, bad));
  CHECK(!l4.matches(0, bad));
  bad = make_frame(IPPROTO_TCP);
  bad.payload[32] = 0xF0;  // TCP data offset exceeds segment length.
  l4.dst_port = 80;
  CHECK(!l4.matches(0, bad));

  l4.dst_port = 53;
  for (uint16_t flags : {0x2000, 0x0001}) {
    auto fragment = frame;
    write_u16_be(fragment.payload.data() + 6, flags);
    repair_ip_checksum(fragment.payload);
    CHECK(l3.matches(0, fragment));
    CHECK(!l4.matches(0, fragment));
  }
}

static void test_priority_and_management() {
  FlowTable table;
  const auto frame = make_frame();
  CHECK(!table.process(0, frame).matched);
  table.add(rule(1, 0, {Drop{}}));
  auto specific = rule(2, 100, {Forward{1}});
  specific.match.dst_port = 53;
  table.add(specific);
  table.add(rule(3, 100, {Forward{2}}));
  auto result = table.process(0, frame);
  CHECK(result.matched && !result.dropped && result.entry_id == 2);
  CHECK(result.outputs.size() == 1 && result.outputs[0].port == 1);
  CHECK(table.find(1)->packet_count == 0);
  CHECK(table.find(2)->packet_count == 1);
  CHECK(table.process(0, make_frame(IPPROTO_TCP)).entry_id == 3);
  specific.actions = {Forward{3}};
  specific.packet_count = 999;
  table.add(specific);
  CHECK(table.size() == 3 && table.entries().size() == 3);
  CHECK(table.find(2)->packet_count == 0);
  CHECK(table.process(0, frame).outputs[0].port == 3);  // Replacement retains tie order.
  CHECK(table.remove(2));
  CHECK(!table.remove(2) && !table.find(2));
  CHECK(table.process(0, frame).entry_id == 3);
  CHECK(table.remove(3));
  result = table.process(0, frame);
  CHECK(result.entry_id == 1 && result.dropped && !result.action_error);
  table.clear();
  CHECK(table.size() == 0 && !table.process(0, frame).matched);
}

static void test_action_order() {
  FlowTable table;
  const auto frame = make_frame();
  Modify change;
  change.dst_mac = MacAddress::broadcast();
  table.add(rule(1, 0, {Forward{0}, change, Forward{1}, Forward{2}}));
  const auto result = table.process(0, frame);
  CHECK(result.outputs.size() == 3);
  CHECK(result.outputs[0].frame.dst == frame.dst);
  CHECK(result.outputs[1].frame.dst == MacAddress::broadcast());
  CHECK(result.outputs[2].frame.dst == MacAddress::broadcast());
  CHECK(frame.dst != MacAddress::broadcast());  // Caller-owned input is untouched.
  table.add(rule(1, 0, {Forward{0}, Drop{}, Forward{1}}));
  auto drop = table.process(0, frame);
  CHECK(drop.dropped && drop.outputs.empty() && !drop.action_error);
  table.add(rule(1, 0, {}));
  CHECK(table.process(0, frame).dropped);
  table.add(rule(1, 0, {change}));
  CHECK(table.process(0, frame).dropped);
}

static void test_modify_udp() {
  const auto frame = make_frame();
  Modify change;
  change.src_mac = mac("02:00:00:00:00:03");
  change.dst_mac = mac("02:00:00:00:00:04");
  change.src_ip = ip("192.168.0.1");
  change.dst_ip = ip("192.168.0.2");
  change.src_port = 0;
  change.dst_port = 5353;
  FlowTable table;
  table.add(rule(1, 0, {change, Forward{1}}));
  const auto result = table.process(0, frame);
  CHECK(!result.action_error && result.outputs.size() == 1);
  const auto& changed = result.outputs[0].frame;
  CHECK(changed.src == *change.src_mac && changed.dst == *change.dst_mac);
  Ipv4Packet packet;
  CHECK(Ipv4Packet::parse(changed.payload, packet));
  CHECK(packet.src == *change.src_ip && packet.dst == *change.dst_ip);
  UdpDatagram udp;
  CHECK(UdpDatagram::parse(packet.payload, packet.src, packet.dst, udp));
  CHECK(udp.src_port == 0 && udp.dst_port == 5353);
  CHECK(udp.payload == Buffer({'s', 'd', 'n'}));
  CHECK(udp.checksum != 0);
  EthernetFrame parsed;
  CHECK(EthernetFrame::parse(changed.serialize(), parsed));  // New FCS is valid.
  CHECK(parsed.payload == changed.payload);
  CHECK(frame.serialize() == make_frame().serialize());

  // Address-only rewriting must also update the UDP pseudo-header checksum.
  Modify address_only;
  address_only.dst_ip = ip("172.16.0.1");
  table.add(rule(1, 0, {address_only, Forward{1}}));
  auto output = table.process(0, frame).outputs[0].frame;
  CHECK(Ipv4Packet::parse(output.payload, packet));
  CHECK(UdpDatagram::parse(packet.payload, packet.src, packet.dst, udp));
  CHECK(udp.dst_port == 53 && packet.dst == *address_only.dst_ip);

  // UDP checksum value 0 must be encoded as 0xFFFF, not "checksum disabled".
  auto zero_checksum = frame;
  zero_checksum.payload.resize(30);
  write_u16_be(zero_checksum.payload.data() + 2, 30);
  write_u16_be(zero_checksum.payload.data() + 24, 10);
  write_u16_be(zero_checksum.payload.data() + 28, 0);
  repair_ip_checksum(zero_checksum.payload);
  Modify port_only;
  port_only.dst_port = 5353;
  table.add(rule(1, 0, {port_only, Forward{1}}));
  output = table.process(0, zero_checksum).outputs[0].frame;
  const auto complement = read_u16_be(output.payload.data() + 26);
  write_u16_be(zero_checksum.payload.data() + 28, complement);
  output = table.process(0, zero_checksum).outputs[0].frame;
  CHECK(read_u16_be(output.payload.data() + 26) == 0xFFFF);
  CHECK(Ipv4Packet::parse(output.payload, packet));
  CHECK(UdpDatagram::parse(packet.payload, packet.src, packet.dst, udp));
}

static void test_modify_tcp_options_and_padding() {
  auto frame = make_frame(IPPROTO_TCP);
  // Preserve both IPv4 and TCP options without round-tripping through the
  // simplified IPv4 serializer (which only emits a 20-byte header).
  frame.payload.insert(frame.payload.begin() + 20, {1, 1, 1, 0});
  frame.payload[0] = 0x46;
  frame.payload.insert(frame.payload.begin() + 44, {1, 1, 1, 0});
  frame.payload[36] = 0x60;
  write_u16_be(frame.payload.data() + 2, static_cast<uint16_t>(frame.payload.size()));
  repair_ip_checksum(frame.payload);
  // Repair the original TCP checksum after inserting TCP options.
  Buffer pseudo(12 + frame.payload.size() - 24);
  std::copy(frame.payload.begin() + 12, frame.payload.begin() + 20, pseudo.begin());
  pseudo[9] = IPPROTO_TCP;
  write_u16_be(pseudo.data() + 10, static_cast<uint16_t>(frame.payload.size() - 24));
  write_u16_be(frame.payload.data() + 40, 0);
  std::copy(frame.payload.begin() + 24, frame.payload.end(), pseudo.begin() + 12);
  write_u16_be(frame.payload.data() + 40, internet_checksum(pseudo));
  frame.payload.insert(frame.payload.end(), {0xAA, 0xBB});  // Ethernet padding.

  FlowMatch match;
  match.dst_port = 80;
  CHECK(match.matches(0, frame));
  Modify change;
  change.src_ip = ip("172.16.0.1");
  change.dst_ip = ip("172.16.0.2");
  change.src_port = 8080;
  change.dst_port = 0;
  FlowTable table;
  table.add(rule(1, 0, {change, Forward{1}}));
  auto result = table.process(0, frame);
  CHECK(result.outputs.size() == 1 && !result.action_error);
  const auto& output = result.outputs[0].frame;
  CHECK(output.payload.size() == frame.payload.size());
  CHECK(output.payload[20] == 1 && output.payload[23] == 0);
  CHECK(output.payload[44] == 1 && output.payload[47] == 0);
  CHECK(output.payload.back() == 0xBB);
  Ipv4Packet packet;
  TcpSegment tcp;
  CHECK(Ipv4Packet::parse(output.payload, packet));
  CHECK(TcpSegment::parse(packet.payload, packet.src, packet.dst, tcp));
  CHECK(tcp.src_port == 8080 && tcp.dst_port == 0 && tcp.data_offset == 6);
  CHECK(tcp.seq == 100 && tcp.ack == 200 && tcp.flags == TcpSegment::ACK);
  CHECK(tcp.payload == Buffer({'G', 'E', 'T'}));
}

static void test_invalid_modify() {
  auto frame = make_frame();
  Modify ports;
  ports.dst_port = 80;
  FlowTable table;
  table.add(rule(1, 0, {Forward{1}, ports, Forward{2}}));
  auto bad = frame;
  bad.ethertype = ETHERTYPE_ARP;
  auto check_drop = [&](const EthernetFrame& input) {
    auto result = table.process(0, input);
    CHECK(result.matched && result.dropped && result.action_error && result.outputs.empty());
  };
  check_drop(bad);
  check_drop(make_frame(IPPROTO_ICMP));
  for (std::size_t n = 0; n < frame.payload.size(); ++n) {
    bad = frame;
    bad.payload.resize(n);
    check_drop(bad);
  }
  for (uint16_t flags : {0x2000, 0x0001}) {
    bad = frame;
    write_u16_be(bad.payload.data() + 6, flags);
    repair_ip_checksum(bad.payload);
    check_drop(bad);
  }
  // L2 rewrites remain legal for non-IP frames.
  Modify l2;
  l2.src_mac = MacAddress{};
  table.add(rule(1, 0, {l2, Forward{1}}));
  bad.ethertype = ETHERTYPE_ARP;
  CHECK(!table.process(0, bad).action_error);
}

static void test_switch_packet_in() {
  SdnSwitch sw(42, 2);
  PhyLink output;
  CHECK(sw.set_port_tx(1, &output));
  CHECK(!sw.set_port_tx(2, &output));
  const auto raw = make_frame().serialize();
  sw.on_frame(0, raw);  // No implicit learning/flooding without a controller.
  CHECK(output.pending() == 0 && sw.flow_table().size() == 0);
  CHECK(sw.stats().table_misses == 1 && sw.stats().packet_ins == 0);
  int events = 0;
  sw.set_packet_in_handler([&](SdnSwitch& source, const PacketIn& packet) {
    ++events;
    CHECK(&source == &sw && packet.switch_id == 42 && packet.ingress_port == 0);
    CHECK(packet.frame.serialize() == raw);
    auto entry = rule(1, 1, {Forward{1}});
    entry.match.ingress_port = packet.ingress_port;
    CHECK(source.apply_flow_mod({FlowMod::Command::Add, entry}));
  });
  sw.on_frame(0, raw);
  CHECK(events == 1 && output.pending() == 1);  // First packet is replayed exactly once.
  sw.on_frame(0, raw);
  CHECK(events == 1 && output.pending() == 2);
  CHECK(sw.flow_table().find(1)->packet_count == 2);
  Buffer received;
  CHECK(output.receive(received) && received == raw);
  CHECK(output.receive(received) && received == raw);
  auto corrupt = raw;
  corrupt[0] ^= 1;
  sw.on_frame(0, corrupt);
  sw.on_frame(2, raw);
  CHECK(events == 1 && output.pending() == 0);
  CHECK(sw.stats().received == 5 && sw.stats().dropped == 3);
  CHECK(sw.stats().packet_ins == 1 && sw.stats().forwarded == 2);

  // An invalid replacement is rejected atomically; the original remains.
  CHECK(!sw.apply_flow_mod({FlowMod::Command::Add, rule(1, 9, {Forward{2}})}));
  CHECK(sw.flow_table().find(1)->priority == 1);
  CHECK(sw.apply_flow_mod({FlowMod::Command::Delete, rule(1, 0, {})}));
  CHECK(!sw.apply_flow_mod({FlowMod::Command::Delete, rule(1, 0, {})}));
  // An unrelated Flow-Mod must not cause recursive Packet-In or forward the miss.
  sw.set_packet_in_handler([&](SdnSwitch& source, const PacketIn&) {
    ++events;
    auto unrelated = rule(2, 0, {Forward{1}});
    unrelated.match.ingress_port = 1;
    CHECK(source.apply_flow_mod({FlowMod::Command::Add, unrelated}));
    source.set_packet_in_handler({});  // Safe to disconnect from inside a callback.
  });
  sw.on_frame(0, raw);
  CHECK(events == 2 && output.pending() == 0);
  CHECK(sw.apply_flow_mod({FlowMod::Command::Clear, {}}));
  CHECK(sw.flow_table().size() == 0);
  // A disconnected output link drops without inventing a control-plane miss.
  CHECK(sw.apply_flow_mod({FlowMod::Command::Add, rule(3, 0, {Forward{0}})}));
  sw.on_frame(0, raw);
  CHECK(sw.stats().table_misses == 3 && output.pending() == 0);
}

static void test_controller() {
  SdnController controller;
  SdnSwitch first(1, 2), second(2, 2);
  PhyLink first_out, second_out;
  CHECK(first.set_port_tx(1, &first_out));
  CHECK(second.set_port_tx(0, &second_out));
  auto dispatch = [&](SdnSwitch& source, const PacketIn& packet) {
    controller.on_packet_in(source, packet);
  };
  first.set_packet_in_handler(dispatch);
  second.set_packet_in_handler(dispatch);
  const auto frame = make_frame();
  first.on_frame(0, frame.serialize());
  CHECK(controller.packet_in_count() == 1 && controller.flow_mod_count() == 0);
  CHECK(first_out.pending() == 0);
  controller.set_policy([](const PacketIn& packet) -> std::optional<FlowEntry> {
    auto entry = rule(7, 10, {Forward{packet.switch_id == 1 ? 1u : 0u}});
    entry.match.ingress_port = packet.ingress_port;
    entry.match.ethertype = packet.frame.ethertype;
    return entry;
  });
  first.on_frame(0, frame.serialize());
  second.on_frame(1, frame.serialize());
  CHECK(controller.packet_in_count() == 3 && controller.flow_mod_count() == 2);
  CHECK(first_out.pending() == 1 && second_out.pending() == 1);
  CHECK(first.flow_table().size() == 1 && second.flow_table().size() == 1);
  first.on_frame(0, frame.serialize());
  CHECK(controller.packet_in_count() == 3 && first_out.pending() == 2);
  CHECK(!controller.on_packet_in(first, PacketIn{999, 0, frame}));
  CHECK(controller.packet_in_count() == 3);
  CHECK(!controller.send_flow_mod(first, {FlowMod::Command::Add, rule(8, 1, {Forward{2}})}));
  CHECK(controller.flow_mod_count() == 2);
  // Proactive control-plane updates: replace, delete, clear, decline a miss.
  CHECK(controller.send_flow_mod(first, {FlowMod::Command::Add, rule(7, 20, {Drop{}})}));
  first.on_frame(0, frame.serialize());
  CHECK(controller.packet_in_count() == 3 && first_out.pending() == 2);
  CHECK(controller.send_flow_mod(first, {FlowMod::Command::Delete, rule(7, 0, {})}));
  CHECK(controller.send_flow_mod(second, {FlowMod::Command::Clear, {}}));
  controller.set_policy([](const PacketIn&) -> std::optional<FlowEntry> { return std::nullopt; });
  first.on_frame(0, frame.serialize());
  CHECK(controller.packet_in_count() == 4 && controller.flow_mod_count() == 5);
  CHECK(first.flow_table().size() == 0 && second.flow_table().size() == 0);
}

int main() {
  struct Test {
    const char* name;
    void (*run)();
  };
  const Test tests[] = {
      {"L2/L3/L4 wildcard matching", test_matches},
      {"malformed and fragmented packets", test_malformed_and_fragments},
      {"priority and rule management", test_priority_and_management},
      {"ordered and terminal actions", test_action_order},
      {"UDP rewriting and checksums", test_modify_udp},
      {"TCP rewriting, options and padding", test_modify_tcp_options_and_padding},
      {"invalid actions fail closed", test_invalid_modify},
      {"switch Packet-In and first-packet replay", test_switch_packet_in},
      {"centralized multi-switch controller", test_controller},
  };
  int failed = 0;
  for (const auto& test : tests) {
    try {
      test.run();
      std::cout << "PASS: " << test.name << "\n";
    } catch (const std::exception& error) {
      ++failed;
      std::cerr << "FAIL: " << test.name << ": " << error.what() << "\n";
    }
  }
  return failed == 0 ? 0 : 1;
}
