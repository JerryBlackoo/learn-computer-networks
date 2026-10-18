#pragma once

#include "netsim/sdn_switch.hpp"

namespace netsim {

/** In-process centralized control plane, not an OpenFlow wire implementation.
 * The policy returns a rule for the reporting switch, or nullopt to decline.
 * Without a policy, packets fail closed. The same controller can serve many
 * switches through their PacketInHandler callbacks; it stores no switch pointers.
 */
class SdnController {
 public:
  using Policy = std::function<std::optional<FlowEntry>(const PacketIn&)>;

  explicit SdnController(Policy policy = {}) : policy_(std::move(policy)) {}
  void set_policy(Policy policy) { policy_ = std::move(policy); }
  bool on_packet_in(SdnSwitch& source, const PacketIn& packet);
  bool send_flow_mod(SdnSwitch& target, const FlowMod& mod);
  uint64_t packet_in_count() const { return packet_ins_; }
  uint64_t flow_mod_count() const { return flow_mods_; }

 private:
  Policy policy_;
  uint64_t packet_ins_ = 0;
  uint64_t flow_mods_ = 0;
};

}  // namespace netsim
