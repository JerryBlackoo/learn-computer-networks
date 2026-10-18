#include "netsim/routing_algo.hpp"

#include <queue>
#include <set>

namespace netsim {

DistanceVectorNode::DistanceVectorNode(std::string name) : name_(std::move(name)) {
  table_[name_] = Entry{0, name_};
}

void DistanceVectorNode::set_direct(const std::string& neighbor, int cost) {
  neighbors_[neighbor] = cost;
  auto& e = table_[neighbor];
  if (cost < e.cost) {
    e.cost = cost;
    e.next_hop = neighbor;
  }
}

bool DistanceVectorNode::on_update(const DvNeighborUpdate& upd) {
  auto nit = neighbors_.find(upd.from);
  if (nit == neighbors_.end()) return false;
  int link = nit->second;
  bool changed = false;
  for (const auto& [dest, dvia] : upd.distance) {
    if (dest == name_) continue;
    int cand = link + dvia;
    if (cand >= kInfMetric) cand = kInfMetric;
    auto it = table_.find(dest);
    if (it == table_.end() || cand < it->second.cost ||
        (it->second.next_hop == upd.from && cand != it->second.cost)) {
      table_[dest] = Entry{cand, upd.from};
      changed = true;
    }
  }
  return changed;
}

DvNeighborUpdate DistanceVectorNode::advertisement(bool poisoned_reverse) const {
  DvNeighborUpdate out;
  out.from = name_;
  for (const auto& [dest, e] : table_) {
    int c = e.cost;
    if (poisoned_reverse && e.next_hop == dest) {
      // incomplete: poison when advertising to next_hop — caller filters per neighbor
    }
    out.distance[dest] = c;
  }
  // For each neighbor advertisement, poison reverse: if next_hop==neighbor, send inf
  // Callers use advertisement_to(neighbor)
  return out;
}

int DistanceVectorNode::cost_to(const std::string& dest) const {
  auto it = table_.find(dest);
  return it == table_.end() ? kInfMetric : it->second.cost;
}

std::string DistanceVectorNode::next_hop(const std::string& dest) const {
  auto it = table_.find(dest);
  return it == table_.end() ? "" : it->second.next_hop;
}

std::unordered_map<std::string, LsRoute> dijkstra_routes(const std::string& source,
                                                        const std::vector<LsLink>& links) {
  std::unordered_map<std::string, std::vector<std::pair<std::string, int>>> adj;
  std::set<std::string> nodes;
  for (const auto& l : links) {
    adj[l.a].push_back({l.b, l.cost});
    adj[l.b].push_back({l.a, l.cost});
    nodes.insert(l.a);
    nodes.insert(l.b);
  }
  nodes.insert(source);

  std::unordered_map<std::string, int> dist;
  std::unordered_map<std::string, std::string> prev;
  for (const auto& n : nodes) dist[n] = std::numeric_limits<int>::max();
  dist[source] = 0;

  using QE = std::pair<int, std::string>;
  std::priority_queue<QE, std::vector<QE>, std::greater<QE>> pq;
  pq.push({0, source});
  while (!pq.empty()) {
    auto [d, u] = pq.top();
    pq.pop();
    if (d != dist[u]) continue;
    for (const auto& [v, w] : adj[u]) {
      if (dist[u] + w < dist[v]) {
        dist[v] = dist[u] + w;
        prev[v] = u;
        pq.push({dist[v], v});
      }
    }
  }

  std::unordered_map<std::string, LsRoute> out;
  for (const auto& n : nodes) {
    LsRoute r;
    r.cost = dist[n];
    if (n == source) {
      r.next_hop = source;
    } else if (dist[n] == std::numeric_limits<int>::max()) {
      r.next_hop = "";
    } else {
      // Walk back to find first hop from source
      std::string cur = n;
      while (prev.count(cur) && prev[cur] != source) cur = prev[cur];
      r.next_hop = cur;
    }
    out[n] = r;
  }
  return out;
}

}  // namespace netsim
