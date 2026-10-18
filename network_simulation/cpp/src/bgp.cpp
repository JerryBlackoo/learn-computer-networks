#include "netsim/bgp.hpp"

namespace netsim {

BgpSpeaker::BgpSpeaker(AsNumber local_as, uint32_t router_id, Ipv4Address router_ip)
    : local_as_(local_as), router_id_(router_id), router_ip_(router_ip) {}

bool BgpSpeaker::add_peer(Ipv4Address peer_ip, AsNumber peer_as) {
  PeerSession session;
  session.as = peer_as;
  session.is_established = false;
  peers_[peer_ip] = session;
  return true;
}

bool BgpSpeaker::is_ebgp_peer(Ipv4Address peer_ip) const {
  auto it = peers_.find(peer_ip);
  if (it == peers_.end()) return false;
  return it->second.as != local_as_;
}

bool BgpSpeaker::is_peer_established(Ipv4Address peer_ip) const {
  auto it = peers_.find(peer_ip);
  if (it == peers_.end()) return false;
  return it->second.is_established;
}

bool BgpSpeaker::handle_open(Ipv4Address peer_ip, const BgpOpenMessage& open_msg) {
  auto it = peers_.find(peer_ip);
  if (it == peers_.end()) return false;
  if (open_msg.version != 4) return false; // BGP-4 standard
  if (open_msg.my_as != it->second.as) return false; // AS number mismatch

  it->second.is_established = true;
  it->second.hold_time = open_msg.hold_time;
  it->second.peer_router_id = open_msg.bgp_identifier;
  return true;
}

void BgpSpeaker::originate_prefix(const std::string& prefix, int prefix_len, uint32_t local_pref) {
  BgpRoute route;
  route.prefix = prefix;
  route.prefix_len = prefix_len;
  route.as_path = {}; // Originated inside this AS -> empty AS-PATH
  route.next_hop = router_ip_;
  route.local_pref = local_pref;
  route.med = 0;
  route.router_id = router_id_;

  rib_[prefix] = std::move(route);
}

bool BgpSpeaker::is_route_better(const BgpRoute& a, const BgpRoute& b) {
  // Step 1: Highest LOCAL-PREF
  if (a.local_pref != b.local_pref) {
    return a.local_pref > b.local_pref;
  }
  // Step 2: Shortest AS-PATH length
  if (a.as_path.size() != b.as_path.size()) {
    return a.as_path.size() < b.as_path.size();
  }
  // Step 3: Lowest Originating Router-ID (Tie-breaker)
  if (a.router_id != b.router_id) {
    return a.router_id < b.router_id;
  }
  return false;
}

bool BgpSpeaker::handle_update(Ipv4Address peer_ip, const BgpUpdateMessage& update_msg) {
  auto it_peer = peers_.find(peer_ip);
  if (it_peer == peers_.end() || !it_peer->second.is_established) {
    return false;
  }

  // Handle withdrawn routes
  for (const auto& w_prefix : update_msg.withdrawn_routes) {
    rib_.erase(w_prefix);
  }

  // Handle announced routes
  for (const auto& route : update_msg.announced_routes) {
    // 1. Loop Prevention: Drop route if local ASN is found in AS-PATH!
    if (route.contains_as(local_as_)) {
      continue; // Loop detected! Discard this route
    }

    // 2. Best-path selection against current RIB entry
    auto it_rib = rib_.find(route.prefix);
    if (it_rib == rib_.end()) {
      rib_[route.prefix] = route;
    } else {
      if (is_route_better(route, it_rib->second)) {
        it_rib->second = route;
      }
    }
  }

  return true;
}

std::optional<BgpRoute> BgpSpeaker::lookup_best_route(const std::string& prefix) const {
  auto it = rib_.find(prefix);
  if (it != rib_.end()) {
    return it->second;
  }
  return std::nullopt;
}

std::optional<BgpUpdateMessage> BgpSpeaker::create_update_for_peer(Ipv4Address peer_ip,
                                                                   const std::string& prefix) const {
  auto it_peer = peers_.find(peer_ip);
  if (it_peer == peers_.end() || !it_peer->second.is_established) {
    return std::nullopt;
  }

  auto best_opt = lookup_best_route(prefix);
  if (!best_opt.has_value()) {
    return std::nullopt;
  }

  BgpRoute outbound_route = *best_opt;

  if (is_ebgp_peer(peer_ip)) {
    // eBGP Rule: Prepend local ASN to AS-PATH, update NEXT-HOP to local router IP
    outbound_route.as_path.insert(outbound_route.as_path.begin(), local_as_);
    outbound_route.next_hop = router_ip_;
  }
  // iBGP Rule: Preserve AS-PATH and pass LOCAL-PREF unchanged

  BgpUpdateMessage msg;
  msg.announced_routes.push_back(std::move(outbound_route));
  return msg;
}

}  // namespace netsim

