#include "netsim/email.hpp"

#include <algorithm>
#include <sstream>

namespace netsim {

// Helper to trim trailing \r or \n
static std::string trim_crlf(const std::string& s) {
  std::string out = s;
  while (!out.empty() && (out.back() == '\r' || out.back() == '\n')) {
    out.pop_back();
  }
  return out;
}

// ============================================================================
// SMTP Implementation
// ============================================================================
SmtpServer::SmtpServer() : state_(State::WAIT_HELO) {}

std::string SmtpServer::process_line(const std::string& raw_line) {
  std::string line = trim_crlf(raw_line);

  if (state_ == State::READ_DATA) {
    if (line == ".") {
      // End of data transmission
      mail_spool_.push_back(current_msg_);
      current_msg_ = EmailMessage{};
      state_ = State::WAIT_MAIL_FROM;
      return "250 Message accepted for delivery\r\n";
    }
    if (!current_msg_.body.empty()) {
      current_msg_.body += "\n";
    }
    current_msg_.body += line;
    return ""; // Server does not respond during data body input
  }

  // Parse command
  std::string cmd;
  std::string arg;
  auto space_pos = line.find(' ');
  if (space_pos != std::string::npos) {
    cmd = line.substr(0, space_pos);
    arg = line.substr(space_pos + 1);
  } else {
    cmd = line;
  }
  std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);

  if (cmd == "QUIT") {
    state_ = State::QUIT;
    return "221 2.0.0 Service closing transmission channel\r\n";
  }

  if (cmd == "HELO" || cmd == "EHLO") {
    state_ = State::WAIT_MAIL_FROM;
    return "250 Hello, pleased to meet you\r\n";
  }

  if (cmd == "MAIL") {
    // Expected "MAIL FROM:<sender>"
    if (state_ != State::WAIT_MAIL_FROM && state_ != State::WAIT_HELO) {
      return "503 Bad sequence of commands\r\n";
    }
    auto colon = arg.find(':');
    if (colon != std::string::npos) {
      current_msg_.from = arg.substr(colon + 1);
    } else {
      current_msg_.from = arg;
    }
    state_ = State::WAIT_RCPT_TO;
    return "250 2.1.0 Sender OK\r\n";
  }

  if (cmd == "RCPT") {
    // Expected "RCPT TO:<recipient>"
    if (state_ != State::WAIT_RCPT_TO) {
      return "503 Bad sequence of commands\r\n";
    }
    auto colon = arg.find(':');
    if (colon != std::string::npos) {
      current_msg_.to = arg.substr(colon + 1);
    } else {
      current_msg_.to = arg;
    }
    state_ = State::WAIT_DATA;
    return "250 2.1.5 Recipient OK\r\n";
  }

  if (cmd == "DATA") {
    if (state_ != State::WAIT_DATA) {
      return "503 Bad sequence of commands\r\n";
    }
    state_ = State::READ_DATA;
    return "354 Start mail input; end with <CRLF>.<CRLF>\r\n";
  }

  return "500 Syntax error, command unrecognized\r\n";
}

// ============================================================================
// POP3 Implementation
// ============================================================================
Pop3Server::Pop3Server(std::string valid_user, std::string valid_pass)
    : user_(std::move(valid_user)), pass_(std::move(valid_pass)) {}

void Pop3Server::add_email(const EmailMessage& msg) {
  MailItem item;
  item.id = static_cast<int>(mailbox_.size()) + 1;
  item.message = msg;
  item.marked_deleted = false;
  mailbox_.push_back(std::move(item));
}

int Pop3Server::remaining_mail_count() const {
  int count = 0;
  for (const auto& item : mailbox_) {
    if (!item.marked_deleted) count++;
  }
  return count;
}

std::string Pop3Server::process_line(const std::string& raw_line) {
  std::string line = trim_crlf(raw_line);
  std::string cmd;
  std::string arg;
  auto space_pos = line.find(' ');
  if (space_pos != std::string::npos) {
    cmd = line.substr(0, space_pos);
    arg = line.substr(space_pos + 1);
  } else {
    cmd = line;
  }
  std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);

  if (cmd == "QUIT") {
    if (state_ == State::TRANSACTION) {
      // UPDATE state: physically delete marked emails
      mailbox_.erase(
          std::remove_if(mailbox_.begin(), mailbox_.end(),
                         [](const MailItem& it) { return it.marked_deleted; }),
          mailbox_.end());
    }
    return "+OK POP3 server signing off\r\n";
  }

  // --- AUTHORIZATION STATE ---
  if (state_ == State::AUTHORIZATION) {
    if (cmd == "USER") {
      authing_user_ = arg;
      return "+OK User accepted\r\n";
    }
    if (cmd == "PASS") {
      if (authing_user_ == user_ && arg == pass_) {
        state_ = State::TRANSACTION;
        return "+OK Pass accepted, maildrop locked and ready\r\n";
      } else {
        return "-ERR Invalid username or password\r\n";
      }
    }
    return "-ERR Please authenticate first\r\n";
  }

  // --- TRANSACTION STATE ---
  if (state_ == State::TRANSACTION) {
    if (cmd == "STAT") {
      // Returns "+OK <count> <total_bytes>"
      int count = remaining_mail_count();
      size_t bytes = 0;
      for (const auto& it : mailbox_) {
        if (!it.marked_deleted) bytes += it.message.body.size();
      }
      return "+OK " + std::to_string(count) + " " + std::to_string(bytes) + "\r\n";
    }

    if (cmd == "LIST") {
      // Returns list of message sizes
      int count = remaining_mail_count();
      std::string resp = "+OK " + std::to_string(count) + " messages\r\n";
      for (const auto& it : mailbox_) {
        if (!it.marked_deleted) {
          resp += std::to_string(it.id) + " " + std::to_string(it.message.body.size()) + "\r\n";
        }
      }
      resp += ".\r\n";
      return resp;
    }

    if (cmd == "RETR") {
      int id = 0;
      try {
        id = std::stoi(arg);
      } catch (...) {
        return "-ERR Invalid message number\r\n";
      }
      for (const auto& it : mailbox_) {
        if (it.id == id) {
          if (it.marked_deleted) return "-ERR Message deleted\r\n";
          std::string resp = "+OK " + std::to_string(it.message.body.size()) + " octets\r\n";
          resp += "From: " + it.message.from + "\r\n";
          resp += "To: " + it.message.to + "\r\n";
          resp += "\r\n" + it.message.body + "\r\n.\r\n";
          return resp;
        }
      }
      return "-ERR No such message\r\n";
    }

    if (cmd == "DELE") {
      int id = 0;
      try {
        id = std::stoi(arg);
      } catch (...) {
        return "-ERR Invalid message number\r\n";
      }
      for (auto& it : mailbox_) {
        if (it.id == id) {
          if (it.marked_deleted) return "-ERR Message already deleted\r\n";
          it.marked_deleted = true;
          return "+OK Message " + std::to_string(id) + " marked for deletion\r\n";
        }
      }
      return "-ERR No such message\r\n";
    }

    if (cmd == "RSET") {
      for (auto& it : mailbox_) {
        it.marked_deleted = false;
      }
      return "+OK Mailbox reset\r\n";
    }
  }

  return "-ERR Unknown command\r\n";
}

}  // namespace netsim

