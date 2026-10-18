#pragma once

#include "netsim/types.hpp"

#include <optional>
#include <string>
#include <unordered_map>

namespace netsim {

class DnsResolver {
 public:
  void add_record(const std::string& name, Ipv4Address addr);
  std::optional<Ipv4Address> resolve(const std::string& name) const;

  /** Extremely simplified DNS message: QNAME string + A answer. */
  static Buffer encode_query(const std::string& name);
  static Buffer encode_response(const std::string& name, Ipv4Address addr);
  static bool decode_query(const Buffer& buf, std::string& name);
  static bool decode_response(const Buffer& buf, std::string& name, Ipv4Address& addr);

 private:
  std::unordered_map<std::string, Ipv4Address> table_;
};

}  // namespace netsim
