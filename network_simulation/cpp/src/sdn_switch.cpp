#include "netsim/sdn_switch.hpp"

namespace netsim {

SdnSwitch::SdnSwitch(uint64_t id, std::size_t num_ports) : id_(id), port_tx_(num_ports, nullptr) {}

bool SdnSwitch::set_port_tx(std::size_t port, PhyLink* tx) {
  if (port >= port_tx_.size()) return false;
  port_tx_[port] = tx;
  return true;
}

bool SdnSwitch::apply_flow_mod(const FlowMod& mod) {
  switch (mod.command) {
    case FlowMod::Command::Add:
      for (const auto& action : mod.entry.actions) {
        if (const auto* forward = std::get_if<Forward>(&action)) {
          if (forward->port >= port_tx_.size()) return false;
        }
      }
      table_.add(mod.entry);
      return true;
    case FlowMod::Command::Delete:
      return table_.remove(mod.entry.id);
    case FlowMod::Command::Clear:
      table_.clear();
      return true;
  }
  return false;
}

void SdnSwitch::on_frame(std::size_t ingress, const Buffer& raw_frame) {
  ++stats_.received;
  EthernetFrame frame;
  if (ingress >= port_tx_.size() || !EthernetFrame::parse(raw_frame, frame)) {
    ++stats_.dropped;
    return;
  }
  auto result = table_.process(ingress, frame);
  if (!result.matched) {
    ++stats_.table_misses;
    // Copy the callback so it can safely replace/disconnect itself while running.
    auto handler = packet_in_;
    if (handler) {
      ++stats_.packet_ins;
      handler(*this, PacketIn{id_, ingress, frame});
      result = table_.process(ingress, frame);
    }
  }
  bool forwarded = false;
  for (const auto& output : result.outputs) {
    if (port_tx_[output.port]) {
      port_tx_[output.port]->send(output.frame.serialize());
      ++stats_.forwarded;
      forwarded = true;
    }
  }
  if (!forwarded) ++stats_.dropped;
}

}  // namespace netsim
