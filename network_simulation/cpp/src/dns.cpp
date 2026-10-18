#include "netsim/dns.hpp"

#include <algorithm>
#include <cstring>

namespace netsim {

void DnsResolver::add_record(const std::string& name, Ipv4Address addr) { table_[name] = addr; }

std::optional<Ipv4Address> DnsResolver::resolve(const std::string& name) const {
  auto it = table_.find(name);
  if (it == table_.end()) return std::nullopt;
  return it->second;
}

Buffer DnsResolver::encode_query(const std::string& name) {
  Buffer out;
  out.push_back('Q');
  out.insert(out.end(), name.begin(), name.end());
  out.push_back('\0');
  return out;
}

Buffer DnsResolver::encode_response(const std::string& name, Ipv4Address addr) {
  Buffer out;
  out.push_back('R');
  out.insert(out.end(), name.begin(), name.end());
  out.push_back('\0');
  Byte ip[4];
  addr.write_bytes(ip);
  out.insert(out.end(), ip, ip + 4);
  return out;
}

bool DnsResolver::decode_query(const Buffer& buf, std::string& name) {
  if (buf.size() < 2 || buf[0] != 'Q') return false;
  auto z = std::find(buf.begin() + 1, buf.end(), 0);
  if (z == buf.end()) return false;
  name.assign(buf.begin() + 1, z);
  return true;
}

bool DnsResolver::decode_response(const Buffer& buf, std::string& name, Ipv4Address& addr) {
  if (buf.size() < 6 || buf[0] != 'R') return false;
  auto z = std::find(buf.begin() + 1, buf.end(), 0);
  if (z == buf.end() || static_cast<std::size_t>(buf.end() - z) < 5) return false;
  name.assign(buf.begin() + 1, z);
  addr = Ipv4Address::from_bytes(&*(z + 1));
  return true;
}

}  // namespace netsim
