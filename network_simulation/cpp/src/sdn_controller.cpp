#include "netsim/sdn_controller.hpp"

namespace netsim {

bool SdnController::on_packet_in(SdnSwitch& source, const PacketIn& packet) {
  if (packet.switch_id != source.id()) return false;
  ++packet_ins_;
  auto policy = policy_;
  if (!policy) return false;
  auto entry = policy(packet);
  if (!entry) return false;
  return send_flow_mod(source, FlowMod{FlowMod::Command::Add, std::move(*entry)});
}

bool SdnController::send_flow_mod(SdnSwitch& target, const FlowMod& mod) {
  if (!target.apply_flow_mod(mod)) return false;
  ++flow_mods_;
  return true;
}

}  // namespace netsim
