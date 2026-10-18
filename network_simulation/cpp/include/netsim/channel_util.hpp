#pragma once

namespace netsim {

/**
 * Stop-and-wait channel utilization (teaching formula).
 * U = (L/R) / (RTT + L/R) = 1 / (1 + 2a) where a = (prop_delay) / (trans_delay),
 * and RTT ≈ 2 * prop_delay for simple model.
 *
 * @param frame_bits frame length L
 * @param rate_bps transmission rate R
 * @param prop_delay_s one-way propagation delay
 */
inline double stop_and_wait_utilization(double frame_bits, double rate_bps, double prop_delay_s) {
  if (rate_bps <= 0 || frame_bits <= 0) return 0;
  double t_trans = frame_bits / rate_bps;
  double t_prop = prop_delay_s;
  return t_trans / (t_trans + 2.0 * t_prop);
}

/** Go-back-N utilization upper bound with window W (ideal, no errors): min(1, W/(1+2a)). */
inline double gbn_utilization(double frame_bits, double rate_bps, double prop_delay_s, int window) {
  if (window <= 0) return 0;
  double t_trans = frame_bits / rate_bps;
  if (t_trans <= 0) return 0;
  double a = prop_delay_s / t_trans;
  double u = window / (1.0 + 2.0 * a);
  return u > 1.0 ? 1.0 : u;
}

}  // namespace netsim
