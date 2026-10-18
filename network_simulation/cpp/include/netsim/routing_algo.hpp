#pragma once

#include "netsim/types.hpp"

#include <limits>
#include <string>
#include <unordered_map>
#include <vector>

namespace netsim {

constexpr int kInfMetric = 16;  // RIP-style infinity for DV teaching model

struct DvNeighborUpdate {
  std::string from;
  std::unordered_map<std::string, int> distance;  // dest -> cost
};

/**
 * Distance-Vector (Bellman-Ford / RIP-like):
 * each node stores dest -> (cost, next_hop).
 */
class DistanceVectorNode {
 public:
  explicit DistanceVectorNode(std::string name);

  void set_direct(const std::string& neighbor, int cost);
  /** Process one received DV advertisement; returns true if table changed. */
  bool on_update(const DvNeighborUpdate& upd);
  /** Snapshot of advertisements this node would send (poisoned reverse optional). */
  DvNeighborUpdate advertisement(bool poisoned_reverse = false) const;

  int cost_to(const std::string& dest) const;
  std::string next_hop(const std::string& dest) const;
  const std::string& name() const { return name_; }

  struct Entry {
    int cost = kInfMetric;
    std::string next_hop;
  };
  const std::unordered_map<std::string, Entry>& table() const { return table_; }

 private:
  std::string name_;
  std::unordered_map<std::string, int> neighbors_;  // direct link costs
  std::unordered_map<std::string, Entry> table_;
};

struct LsLink {
  std::string a;
  std::string b;
  int cost = 1;
};

/**
 * Link-State (Dijkstra) over undirected weighted graph.
 * Returns dest -> (cost, next_hop from `source`).
 */
struct LsRoute {
  int cost = std::numeric_limits<int>::max();
  std::string next_hop;
};

std::unordered_map<std::string, LsRoute> dijkstra_routes(
    const std::string& source, const std::vector<LsLink>& links);

}  // namespace netsim
