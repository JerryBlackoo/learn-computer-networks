#include "netsim/route.hpp"

#include <algorithm>

namespace netsim {

void RouteTable::add(RouteEntry e) {
  entries_.push_back(e);
  std::sort(entries_.begin(), entries_.end(),
            [](const RouteEntry& a, const RouteEntry& b) { return a.prefix_len > b.prefix_len; });
}

std::optional<RouteEntry> RouteTable::longest_prefix_match(Ipv4Address dst) const {
  for (const auto& e : entries_) {
    uint32_t m = e.mask();
    if ((dst.host_order & m) == (e.network.host_order & m)) {
      return e;
    }
  }
  return std::nullopt;
}

}  // namespace netsim
