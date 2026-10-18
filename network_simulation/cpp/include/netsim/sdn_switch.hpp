#pragma once

#include "netsim/flow_table.hpp"
#include "netsim/phy.hpp"

#include <functional>
#include <utility>

namespace netsim {

struct PacketIn {
  uint64_t switch_id = 0;
  std::size_t ingress_port = 0;
  EthernetFrame frame;
};

struct FlowMod {
  enum class Command { Add, Delete, Clear };
  Command command = Command::Add;
  FlowEntry entry;  // Add: complete rule; Delete: only id is used; Clear: ignored.
};

struct SdnSwitchStats {
  uint64_t received = 0;
  uint64_t table_misses = 0;
  uint64_t packet_ins = 0;
  uint64_t forwarded = 0;  // Number of output copies submitted to attached links.
  uint64_t dropped = 0;    // Input packets with no output (including bad ingress/CRC).
};

/** Pure execution plane. No MAC learning, flooding, ARP, or route decisions.
 * Ports are zero-based; explicit Forward to the ingress port is allowed.
 * Links and objects captured by handlers must outlive their use by the switch.
 */
class SdnSwitch {
 public:
  using PacketInHandler = std::function<void(SdnSwitch&, const PacketIn&)>;

  SdnSwitch(uint64_t id, std::size_t num_ports);
  uint64_t id() const { return id_; }
  bool set_port_tx(std::size_t port, PhyLink* tx);
  void set_packet_in_handler(PacketInHandler handler) { packet_in_ = std::move(handler); }
  bool apply_flow_mod(const FlowMod& mod);
  const FlowTable& flow_table() const { return table_; }
  const SdnSwitchStats& stats() const { return stats_; }

  // A miss emits at most one synchronous Packet-In. Retry the held packet once
  // after the handler returns; if still unmatched, drop without another event.
  void on_frame(std::size_t ingress, const Buffer& raw_frame);

 private:
  uint64_t id_;
  std::vector<PhyLink*> port_tx_;
  FlowTable table_;
  PacketInHandler packet_in_;
  SdnSwitchStats stats_;
};

}  // namespace netsim
