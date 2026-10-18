#pragma once

#include "netsim/types.hpp"

#include <optional>
#include <string>
#include <vector>

namespace netsim {

struct RouteEntry {
  Ipv4Address network;
  uint8_t prefix_len = 0;          // CIDR length
  Ipv4Address next_hop;            // 0.0.0.0 => deliver on iface
  std::string iface;
  int metric = 1;

  uint32_t mask() const {
    return prefix_len == 0 ? 0u : (0xFFFFFFFFu << (32 - prefix_len));
  }
};

class RouteTable {
 public:
  void add(RouteEntry e);
  /** Longest prefix match; returns nullopt if no route. */
  std::optional<RouteEntry> longest_prefix_match(Ipv4Address dst) const;
  const std::vector<RouteEntry>& entries() const { return entries_; }

 private:
  std::vector<RouteEntry> entries_;
};

}  // namespace netsim
