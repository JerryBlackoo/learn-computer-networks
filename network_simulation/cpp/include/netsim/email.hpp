#pragma once

#include "netsim/types.hpp"

#include <string>
#include <vector>
#include <sstream>
#include <unordered_map>

namespace netsim {

struct EmailMessage {
  std::string from;
  std::string to;
  std::string subject;
  std::string body;
};

// ============================================================================
// SMTP (Simple Mail Transfer Protocol - RFC 5321)
// Push protocol, client uses commands: HELO, MAIL FROM, RCPT TO, DATA, QUIT.
// ============================================================================
class SmtpServer {
 public:
  enum class State {
    INIT,
    WAIT_HELO,
    WAIT_MAIL_FROM,
    WAIT_RCPT_TO,
    WAIT_DATA,
    READ_DATA,
    QUIT
  };

  SmtpServer();

  std::string get_greeting() const { return "220 smtp.simulation.local Service ready\r\n"; }

  // Process incoming line from client and return SMTP response string
  std::string process_line(const std::string& line);

  const std::vector<EmailMessage>& get_mail_spool() const { return mail_spool_; }
  size_t mail_count() const { return mail_spool_.size(); }

 private:
  State state_ = State::WAIT_HELO;
  EmailMessage current_msg_;
  std::vector<EmailMessage> mail_spool_;
};

// ============================================================================
// POP3 (Post Office Protocol v3 - RFC 1939)
// Pull protocol, client uses commands: USER, PASS, STAT, LIST, RETR, DELE, QUIT.
// ============================================================================
class Pop3Server {
 public:
  enum class State {
    AUTHORIZATION,
    TRANSACTION,
    UPDATE
  };

  struct MailItem {
    int id;
    EmailMessage message;
    bool marked_deleted = false;
  };

  Pop3Server(std::string valid_user, std::string valid_pass);

  void add_email(const EmailMessage& msg);

  std::string get_greeting() const { return "+OK POP3 simulation server ready\r\n"; }

  // Process POP3 command line and return response string
  std::string process_line(const std::string& line);

  int remaining_mail_count() const;

 private:
  State state_ = State::AUTHORIZATION;
  std::string user_;
  std::string pass_;
  std::string authing_user_;
  std::vector<MailItem> mailbox_;
};

}  // namespace netsim

