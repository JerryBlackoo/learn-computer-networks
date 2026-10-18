#pragma once

#include <cmath>

namespace netsim {

/** Nyquist: C = 2 * W * log2(M) bit/s (ideal noiseless channel). */
inline double nyquist_bitrate(double bandwidth_hz, int signal_levels) {
  if (bandwidth_hz <= 0 || signal_levels < 2) return 0;
  return 2.0 * bandwidth_hz * std::log2(static_cast<double>(signal_levels));
}

/** Shannon: C = W * log2(1 + S/N) bit/s. snr_db = 10*log10(S/N). */
inline double shannon_bitrate(double bandwidth_hz, double snr_db) {
  if (bandwidth_hz <= 0) return 0;
  double snr = std::pow(10.0, snr_db / 10.0);
  return bandwidth_hz * std::log2(1.0 + snr);
}

}  // namespace netsim
