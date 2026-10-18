#pragma once

#include "netsim/types.hpp"

#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace netsim {

using AsNumber = uint32_t;

/**
 * RFC 4271 BGP-4 Message Types
 */
enum class BgpMessageType : uint8_t {
  OPEN         = 1,
  UPDATE       = 2,
  NOTIFICATION = 3,
  KEEPALIVE    = 4
};

/**
 * BGP Route with Path Attributes (RFC 4271)
 */
struct BgpRoute {
  std::string prefix;            // e.g. "10.1.0.0/16"
  int prefix_len = 24;
  std::vector<AsNumber> as_path; // AS-PATH attribute (sequence of traversed ASes)
  Ipv4Address next_hop;          // NEXT-HOP attribute
  uint32_t local_pref = 100;     // LOCAL-PREF attribute (policy priority, higher is better)
  uint32_t med = 0;              // Multi-Exit Discriminator
  uint32_t router_id = 0;        // Originating BGP Router-ID (Tie-breaker)

  bool contains_as(AsNumber asn) const {
    return std::find(as_path.begin(), as_path.end(), asn) != as_path.end();
  }

  size_t as_path_length() const { return as_path.size(); }
};

struct BgpOpenMessage {
  uint8_t version = 4;
  AsNumber my_as = 0;
  uint16_t hold_time = 180;
  uint32_t bgp_identifier = 0; // Router-ID in uint32
};

struct BgpUpdateMessage {
  std::vector<std::string> withdrawn_routes;
  std::vector<BgpRoute> announced_routes;
};

/**
 * Teaching BGP-4 Speaker Model:
 * - Runs on TCP port 179
 * - Manages eBGP (inter-AS) and iBGP (intra-AS) sessions
 * - OPEN, UPDATE, KEEPALIVE, NOTIFICATION message exchange
 * - AS-PATH loop detection: drops routes containing local ASN
 * - BGP Decision Process (Route Selection):
 *   1. Highest LOCAL-PREF
 *   2. Shortest AS-PATH length
 *   3. Lowest Router-ID
 */
class BgpSpeaker {
 public:
  BgpSpeaker(AsNumber local_as, uint32_t router_id, Ipv4Address router_ip);

  AsNumber local_as() const { return local_as_; }
  uint32_t router_id() const { return router_id_; }
  Ipv4Address router_ip() const { return router_ip_; }

  // Configure peer (eBGP if peer_as != local_as, iBGP if peer_as == local_as)
  bool add_peer(Ipv4Address peer_ip, AsNumber peer_as);
  bool is_ebgp_peer(Ipv4Address peer_ip) const;
  bool is_peer_established(Ipv4Address peer_ip) const;

  // Process OPEN message from peer
  bool handle_open(Ipv4Address peer_ip, const BgpOpenMessage& open_msg);

  // Originate a local network prefix into BGP
  void originate_prefix(const std::string& prefix, int prefix_len, uint32_t local_pref = 100);

  // Process incoming UPDATE message:
  // Detects AS-PATH loops, applies policy, and runs best-path selection
  bool handle_update(Ipv4Address peer_ip, const BgpUpdateMessage& update_msg);

  // Create an UPDATE message customized for a specific peer:
  // - If eBGP: prepends local ASN to AS-PATH, sets NEXT-HOP to local IP
  // - If iBGP: preserves AS-PATH and passes LOCAL-PREF
  std::optional<BgpUpdateMessage> create_update_for_peer(Ipv4Address peer_ip, const std::string& prefix) const;

  // Lookup best route in BGP Routing Information Base (RIB)
  std::optional<BgpRoute> lookup_best_route(const std::string& prefix) const;
  size_t rib_size() const { return rib_.size(); }

  // The 4-step BGP decision process comparator: returns true if route A is preferred over route B
  static bool is_route_better(const BgpRoute& a, const BgpRoute& b);

 private:
  AsNumber local_as_;
  uint32_t router_id_;
  Ipv4Address router_ip_;

  struct PeerSession {
    AsNumber as = 0;
    bool is_established = false;
    uint16_t hold_time = 180;
    uint32_t peer_router_id = 0;
  };

  struct IpHash {
    std::size_t operator()(const Ipv4Address& ip) const noexcept {
      return std::hash<uint32_t>{}(ip.host_order);
    }
  };

  std::unordered_map<Ipv4Address, PeerSession, IpHash> peers_;
  // Prefix -> Best BgpRoute
  std::unordered_map<std::string, BgpRoute> rib_;
};

}  // namespace netsim

