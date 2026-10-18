#include "netsim/flow_table.hpp"

#include "netsim/checksum.hpp"
#include "netsim/ipv4.hpp"

#include <algorithm>
#include <utility>

namespace netsim {
namespace {

struct Ipv4View {
  std::size_t header_size = 0;
  std::size_t total_size = 0;
  std::size_t transport_size = 0;
  uint8_t protocol = 0;
  bool fragmented = false;
};

// Inspect bytes in place so rewriting does not discard IP/TCP options or padding.
bool inspect_ipv4(const EthernetFrame& frame, Ipv4View& view) {
  const auto& ip = frame.payload;
  if (frame.ethertype != ETHERTYPE_IPV4 || ip.size() < 20 || (ip[0] >> 4) != 4)
    return false;
  view.header_size = (ip[0] & 0x0F) * 4;
  view.total_size = read_u16_be(ip.data() + 2);
  if (view.header_size < 20 || view.header_size > ip.size() ||
      view.total_size < view.header_size || view.total_size > ip.size())
    return false;
  if (internet_checksum(ip.data(), view.header_size) != 0) return false;
  view.protocol = ip[9];
  view.fragmented = (read_u16_be(ip.data() + 6) & 0x3FFF) != 0;
  return true;
}

bool inspect_transport(const EthernetFrame& frame, Ipv4View& view) {
  if (view.fragmented) return false;
  const auto available = view.total_size - view.header_size;
  const auto* transport = frame.payload.data() + view.header_size;
  if (view.protocol == IPPROTO_UDP) {
    if (available < 8) return false;
    view.transport_size = read_u16_be(transport + 4);
    return view.transport_size >= 8 && view.transport_size <= available;
  }
  if (view.protocol == IPPROTO_TCP) {
    if (available < 20) return false;
    const std::size_t header = (transport[12] >> 4) * 4;
    view.transport_size = available;
    return header >= 20 && header <= available;
  }
  return false;
}

template <typename T>
bool matches_field(const std::optional<T>& expected, const T& value) {
  return !expected || *expected == value;
}

bool modify_frame(EthernetFrame& frame, const Modify& action) {
  const bool rewrite_ip = action.src_ip || action.dst_ip;
  const bool rewrite_ports = action.src_port || action.dst_port;
  if (rewrite_ip || rewrite_ports) {
    Ipv4View view;
    if (!inspect_ipv4(frame, view) || view.fragmented) return false;
    const bool tcp_udp = view.protocol == IPPROTO_TCP || view.protocol == IPPROTO_UDP;
    if ((tcp_udp || rewrite_ports) && !inspect_transport(frame, view)) return false;

    auto& ip = frame.payload;
    if (action.src_ip) action.src_ip->write_bytes(ip.data() + 12);
    if (action.dst_ip) action.dst_ip->write_bytes(ip.data() + 16);
    if (tcp_udp) {
      auto* transport = ip.data() + view.header_size;
      if (action.src_port) write_u16_be(transport, *action.src_port);
      if (action.dst_port) write_u16_be(transport + 2, *action.dst_port);
      const std::size_t checksum_offset = view.protocol == IPPROTO_UDP ? 6 : 16;
      write_u16_be(transport + checksum_offset, 0);
      Buffer pseudo(12 + view.transport_size);
      std::copy(ip.begin() + 12, ip.begin() + 20, pseudo.begin());
      pseudo[9] = view.protocol;
      write_u16_be(pseudo.data() + 10, static_cast<uint16_t>(view.transport_size));
      std::copy(transport, transport + view.transport_size, pseudo.begin() + 12);
      uint16_t checksum = internet_checksum(pseudo);
      if (view.protocol == IPPROTO_UDP && checksum == 0) checksum = 0xFFFF;
      write_u16_be(transport + checksum_offset, checksum);
    }
    write_u16_be(ip.data() + 10, 0);
    write_u16_be(ip.data() + 10, internet_checksum(ip.data(), view.header_size));
  }
  if (action.src_mac) frame.src = *action.src_mac;
  if (action.dst_mac) frame.dst = *action.dst_mac;
  return true;
}

}  // namespace

bool FlowMatch::matches(std::size_t ingress, const EthernetFrame& frame) const {
  if (!matches_field(ingress_port, ingress) || !matches_field(src_mac, frame.src) ||
      !matches_field(dst_mac, frame.dst) || !matches_field(ethertype, frame.ethertype))
    return false;
  if (!src_ip && !dst_ip && !ip_protocol && !src_port && !dst_port) return true;
  Ipv4View view;
  if (!inspect_ipv4(frame, view)) return false;
  if (!matches_field(src_ip, Ipv4Address::from_bytes(frame.payload.data() + 12)) ||
      !matches_field(dst_ip, Ipv4Address::from_bytes(frame.payload.data() + 16)) ||
      !matches_field(ip_protocol, view.protocol))
    return false;
  if (!src_port && !dst_port) return true;
  if (!inspect_transport(frame, view)) return false;
  const auto* transport = frame.payload.data() + view.header_size;
  return matches_field(src_port, read_u16_be(transport)) &&
         matches_field(dst_port, read_u16_be(transport + 2));
}

void FlowTable::add(FlowEntry entry) {
  entry.packet_count = 0;
  for (auto& current : entries_) {
    if (current.id == entry.id) {
      current = std::move(entry);
      return;
    }
  }
  entries_.push_back(std::move(entry));
}

bool FlowTable::remove(uint64_t id) {
  const auto it = std::find_if(entries_.begin(), entries_.end(),
                               [id](const FlowEntry& entry) { return entry.id == id; });
  if (it == entries_.end()) return false;
  entries_.erase(it);
  return true;
}

void FlowTable::clear() { entries_.clear(); }

const FlowEntry* FlowTable::find(uint64_t id) const {
  for (const auto& entry : entries_) {
    if (entry.id == id) return &entry;
  }
  return nullptr;
}

FlowResult FlowTable::process(std::size_t ingress, const EthernetFrame& frame) {
  FlowEntry* selected = nullptr;
  for (auto& entry : entries_) {
    if ((!selected || entry.priority > selected->priority) && entry.match.matches(ingress, frame))
      selected = &entry;
  }
  FlowResult result;
  if (!selected) return result;
  ++selected->packet_count;
  result.matched = true;
  result.entry_id = selected->id;
  EthernetFrame current = frame;
  for (const auto& action : selected->actions) {
    if (const auto* forward = std::get_if<Forward>(&action)) {
      result.outputs.push_back({forward->port, current});
    } else if (std::holds_alternative<Drop>(action)) {
      result.outputs.clear();
      break;
    } else if (!modify_frame(current, std::get<Modify>(action))) {
      result.action_error = true;
      result.outputs.clear();
      break;
    }
  }
  result.dropped = result.outputs.empty();
  return result;
}

}  // namespace netsim
