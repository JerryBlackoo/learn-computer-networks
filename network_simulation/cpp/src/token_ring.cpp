#include "netsim/token_ring.hpp"

#include <stdexcept>

namespace netsim {

TokenRing::TokenRing(int num_stations)
    : num_stations_(num_stations),
      station_queues_(num_stations),
      station_received_(num_stations) {
  if (num_stations < 2) {
    throw std::invalid_argument("Token ring must have at least 2 stations");
  }
  // Initialize in-flight free token at station 0
  in_flight_.is_token = true;
  in_flight_.is_busy = false;
}

void TokenRing::enqueue(int src_node, int dst_node, Buffer data) {
  if (src_node < 0 || src_node >= num_stations_ || dst_node < 0 || dst_node >= num_stations_) {
    throw std::out_of_range("Station index out of range");
  }
  RingFrame f;
  f.is_token = false;
  f.is_busy = true;
  f.src_node = src_node;
  f.dst_node = dst_node;
  f.payload = std::move(data);
  station_queues_[src_node].push_back(std::move(f));
}

const std::vector<Buffer>& TokenRing::get_received(int node) const {
  if (node < 0 || node >= num_stations_) {
    throw std::out_of_range("Station index out of range");
  }
  return station_received_[node];
}

bool TokenRing::step() {
  if (!has_in_flight_data_) {
    // Current station holds or receives the free token
    tokens_passed_++;
    // Does current station have data to send?
    if (!station_queues_[token_pos_].empty()) {
      // Capture free token -> send data frame downstream
      in_flight_ = std::move(station_queues_[token_pos_].front());
      station_queues_[token_pos_].pop_front();
      in_flight_.is_busy = true;
      has_in_flight_data_ = true;
    } else {
      // No data to send -> pass free token to next downstream station
      token_pos_ = (token_pos_ + 1) % num_stations_;
      return true;
    }
  }

  // An in-flight data frame is travelling down the ring to the next station
  int next_station = (token_pos_ + 1) % num_stations_;
  token_pos_ = next_station;

  if (in_flight_.dst_node == token_pos_) {
    // Destination station recognizes address and copies frame
    in_flight_.status.address_recognized = true;
    in_flight_.status.frame_copied = true;
    station_received_[token_pos_].push_back(in_flight_.payload);
  }

  if (in_flight_.src_node == token_pos_) {
    // Frame has travelled full circle back to source station!
    // Source station strips frame and releases a new free token
    if (in_flight_.status.address_recognized && in_flight_.status.frame_copied) {
      frames_delivered_++;
    }
    has_in_flight_data_ = false;
    in_flight_.is_token = true;
    in_flight_.is_busy = false;
    in_flight_.payload.clear();
    // Pass free token downstream to the next station
    token_pos_ = (token_pos_ + 1) % num_stations_;
  }

  return true;
}

int TokenRing::run_until_idle(int max_steps) {
  int steps = 0;
  while (steps < max_steps) {
    bool has_pending = has_in_flight_data_;
    for (const auto& q : station_queues_) {
      if (!q.empty()) {
        has_pending = true;
        break;
      }
    }
    if (!has_pending) {
      break;
    }
    step();
    steps++;
  }
  return steps;
}

}  // namespace netsim

