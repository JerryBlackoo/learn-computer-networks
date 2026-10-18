#include "netsim/http.hpp"

#include <sstream>

namespace netsim {

std::string HttpRequest::serialize() const {
  std::ostringstream oss;
  oss << method << " " << path << " HTTP/1.0\r\n";
  if (!host.empty()) oss << "Host: " << host << "\r\n";
  oss << "Content-Length: " << body.size() << "\r\n\r\n";
  oss << body;
  return oss.str();
}

bool HttpRequest::parse(const std::string& raw, HttpRequest& out) {
  auto pos = raw.find("\r\n\r\n");
  std::string head = pos == std::string::npos ? raw : raw.substr(0, pos);
  std::string body = pos == std::string::npos ? "" : raw.substr(pos + 4);
  std::istringstream iss(head);
  std::string version;
  if (!(iss >> out.method >> out.path >> version)) return false;
  out.body = body;
  out.host.clear();
  std::string line;
  std::getline(iss, line);  // rest of request line
  while (std::getline(iss, line)) {
    if (!line.empty() && line.back() == '\r') line.pop_back();
    if (line.rfind("Host:", 0) == 0) {
      out.host = line.substr(5);
      while (!out.host.empty() && out.host.front() == ' ') out.host.erase(out.host.begin());
    }
  }
  return true;
}

std::string HttpResponse::serialize() const {
  std::ostringstream oss;
  oss << "HTTP/1.0 " << status << " " << reason << "\r\n";
  oss << "Content-Type: " << content_type << "\r\n";
  oss << "Content-Length: " << body.size() << "\r\n\r\n";
  oss << body;
  return oss.str();
}

bool HttpResponse::parse(const std::string& raw, HttpResponse& out) {
  auto pos = raw.find("\r\n\r\n");
  std::string head = pos == std::string::npos ? raw : raw.substr(0, pos);
  out.body = pos == std::string::npos ? "" : raw.substr(pos + 4);
  std::istringstream iss(head);
  std::string http, reason;
  if (!(iss >> http >> out.status)) return false;
  std::getline(iss, reason);
  if (!reason.empty() && reason.front() == ' ') reason.erase(reason.begin());
  if (!reason.empty() && reason.back() == '\r') reason.pop_back();
  out.reason = reason;
  return true;
}

}  // namespace netsim
