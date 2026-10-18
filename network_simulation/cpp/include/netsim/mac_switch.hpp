#pragma once

#include "netsim/ethernet.hpp"
#include "netsim/phy.hpp"
#include "netsim/types.hpp"

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace netsim {

/**
 * Learning Ethernet bridge/switch (self-learning):
 * - Learn src MAC -> ingress port
 * - Forward known dst to port; unknown/broadcast flood (except ingress)
 */
class MacLearningSwitch {
 public:
  explicit MacLearningSwitch(std::size_t num_ports);

  /** Attach unidirectional TX toward a host (switch port TX = host RX). */
  void set_port_tx(std::size_t port, PhyLink* tx_to_host);

  /** Receive a raw Ethernet frame on `ingress` and forward accordingly. */
  void on_frame(std::size_t ingress, const Buffer& raw_frame);

  std::optional<std::size_t> lookup(const MacAddress& mac) const;
  std::size_t table_size() const { return table_.size(); }

 private:
  struct Key {
    std::array<Byte, 6> b{};
    bool operator==(const Key& o) const { return b == o.b; }
  };
  struct Hash {
    std::size_t operator()(const Key& k) const {
      std::size_t h = 0;
      for (auto x : k.b) h = h * 131 + x;
      return h;
    }
  };
  static Key key_of(const MacAddress& m) {
    Key k;
    k.b = m.bytes;
    return k;
  }

  std::vector<PhyLink*> port_tx_;
  std::unordered_map<Key, std::size_t, Hash> table_;
};

}  // namespace netsim
