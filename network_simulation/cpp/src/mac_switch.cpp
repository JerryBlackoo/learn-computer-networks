#include "netsim/mac_switch.hpp"

namespace netsim {

MacLearningSwitch::MacLearningSwitch(std::size_t num_ports) : port_tx_(num_ports, nullptr) {}

void MacLearningSwitch::set_port_tx(std::size_t port, PhyLink* tx_to_host) {
  if (port < port_tx_.size()) port_tx_[port] = tx_to_host;
}

std::optional<std::size_t> MacLearningSwitch::lookup(const MacAddress& mac) const {
  auto it = table_.find(key_of(mac));
  if (it == table_.end()) return std::nullopt;
  return it->second;
}

void MacLearningSwitch::on_frame(std::size_t ingress, const Buffer& raw_frame) {
  EthernetFrame frame;
  if (!EthernetFrame::parse(raw_frame, frame)) return;
  if (ingress >= port_tx_.size()) return;

  table_[key_of(frame.src)] = ingress;

  auto send_to = [&](std::size_t p) {
    if (p == ingress) return;
    if (p < port_tx_.size() && port_tx_[p]) port_tx_[p]->send(raw_frame);
  };

  bool is_bcast = true;
  for (auto b : frame.dst.bytes) {
    if (b != 0xFF) {
      is_bcast = false;
      break;
    }
  }

  if (is_bcast) {
    for (std::size_t p = 0; p < port_tx_.size(); ++p) send_to(p);
    return;
  }

  auto known = lookup(frame.dst);
  if (known) {
    send_to(*known);
  } else {
    for (std::size_t p = 0; p < port_tx_.size(); ++p) send_to(p);
  }
}

}  // namespace netsim
