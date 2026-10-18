#pragma once

#include "netsim/types.hpp"

#include <string>

namespace netsim {

struct HttpRequest {
  std::string method = "GET";
  std::string path = "/";
  std::string host;
  std::string body;

  std::string serialize() const;
  static bool parse(const std::string& raw, HttpRequest& out);
};

struct HttpResponse {
  int status = 200;
  std::string reason = "OK";
  std::string content_type = "text/plain";
  std::string body;

  std::string serialize() const;
  static bool parse(const std::string& raw, HttpResponse& out);
};

}  // namespace netsim
