#pragma once

#include "netsim/ethernet.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <variant>
#include <vector>

namespace netsim {

/** Fields are ANDed; an unset optional is a wildcard (zero is an exact value).
 * L3 requires valid IPv4; L4 requires an unfragmented TCP/UDP header.
 */
struct FlowMatch {
  std::optional<std::size_t> ingress_port;
  std::optional<MacAddress> src_mac;
  std::optional<MacAddress> dst_mac;
  std::optional<uint16_t> ethertype;
  std::optional<Ipv4Address> src_ip;
  std::optional<Ipv4Address> dst_ip;
  std::optional<uint8_t> ip_protocol;
  std::optional<uint16_t> src_port;
  std::optional<uint16_t> dst_port;

  bool matches(std::size_t ingress, const EthernetFrame& frame) const;
};

struct Forward {
  std::size_t port;
};

struct Drop {};

/** Rewrite existing fields only; TCP/UDP and IPv4 checksums are repaired.
 * L3/L4 rewrites reject fragmented or malformed IPv4 packets.
 */
struct Modify {
  std::optional<MacAddress> src_mac;
  std::optional<MacAddress> dst_mac;
  std::optional<Ipv4Address> src_ip;
  std::optional<Ipv4Address> dst_ip;
  std::optional<uint16_t> src_port;
  std::optional<uint16_t> dst_port;
};

using FlowAction = std::variant<Forward, Drop, Modify>;

struct FlowEntry {
  uint64_t id = 0;
  uint16_t priority = 0;
  FlowMatch match;
  std::vector<FlowAction> actions;
  uint64_t packet_count = 0;
};

struct FlowOutput {
  std::size_t port;
  EthernetFrame frame;
};

struct FlowResult {
  bool matched = false;
  bool dropped = false;
  bool action_error = false;
  std::optional<uint64_t> entry_id;
  std::vector<FlowOutput> outputs;
};

/** Single-table, deterministic pipeline; no learning or routing policy.
 * Highest priority wins; ties retain insertion order. Forward snapshots the
 * current frame. Drop or an invalid Modify cancels ALL pending outputs.
 * Empty/Modify-only action lists drop. No second lookup follows a Modify.
 */
class FlowTable {
 public:
  // Add or replace by id, preserving tie order and resetting packet_count.
  void add(FlowEntry entry);
  bool remove(uint64_t id);
  void clear();
  const FlowEntry* find(uint64_t id) const;
  const std::vector<FlowEntry>& entries() const { return entries_; }
  std::size_t size() const { return entries_.size(); }

  FlowResult process(std::size_t ingress, const EthernetFrame& frame);

 private:
  std::vector<FlowEntry> entries_;
};

}  // namespace netsim
