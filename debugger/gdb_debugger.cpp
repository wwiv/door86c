#include "debugger/gdb_debugger.h"

#include "core/log.h"
#include "core/net.h"
#include "core/os.h"
#include "core/socket_connection.h"
#include "core/strings.h"
#include "fmt/format.h"

#include <chrono>
#include <iostream>
#include <set>
#include <string>

#ifdef _WIN32
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")
#else
#include <errno.h>
#include <unistd.h>
#endif

namespace door86::dbg {

using namespace wwiv::strings;

GdbRemote::~GdbRemote() {
  closesocket(sock_);
  sock_ = INVALID_SOCKET;
}

bool GdbRemote::ack() {
  ::send(sock_, "+", 1, 0);
  return true;
}

bool GdbRemote::wait_ack_or_nak() {
  while (true) {
    char ch{0};
    int num_read = ::recv(sock_, &ch, 1, 0);
    if (num_read < 0) {
      sock_ = INVALID_SOCKET;
      return false;
    } else if (num_read == 0) {
      sock_ = INVALID_SOCKET;
      return false;
    }
    switch (ch) {
    case '-': return false;
    case '+': return true;
    default: continue;
    }
  }
}

bool GdbRemote::nak() {
  ::send(sock_, "-", 1, 0);
  return true;
}

bool GdbRemote::response(const std::string& resp) {
  unsigned checksum = 0;
  for (const auto ch : resp) {
    checksum += static_cast<uint8_t>(ch);
  }
  checksum &= 0xff;
  for (int count = 0; count < 10; count++) {
    const auto line = fmt::format("${}#{:02X}", resp, checksum);
    LOG(INFO) << "GdbRemote::response: " << line;
    ::send(sock_, line.c_str(), line.size(), 0);
    if (wait_ack_or_nak()) {
      return true;
    }
    wwiv::os::sleep_for(std::chrono::seconds(1));
  }
  return false;
}

char wait_for_char(SOCKET sock, std::set<char> expected) {
  for (;;) {
    char ch;
    int num_read = ::recv(sock, &ch, 1, 0);
    if (num_read <= 0) {
      // TODO(rushfan): Exception?
      return {};
    }
    if (auto pos = expected.find(ch); pos != std::end(expected)) {
      return ch;
    }
  }
}

bool validate_checksum(const std::string& s, const std::string& cs) {
  try {
    const auto chk = std::stoul(cs, nullptr, 16);
    unsigned int cur{0};
    for (const auto ch : s) {
      cur += static_cast<unsigned char>(ch);
    }
    cur &= 0xff;
    return cur == chk;
  } catch (std::invalid_argument&) {
    return false;
  }
}

std::string GdbRemote::request() {
  // Wait to start of request
  if (wait_for_char(sock_, {'$'}) != '$') {
    return {};
  }
  std::string line;
  line.reserve(255);
  for (;;) {
    char ch;
    int num_read = ::recv(sock_, &ch, 1, 0);
    if (num_read <= 0) {
      // TODO(rushfan): Exception?
      return {};
    }
    if (ch == '#') {
      // switch to checksum
      char cs[2];
      if (const auto cs_read = ::recv(sock_, cs, 2, 0); cs_read == 2) {
        if (validate_checksum(line, cs)) {
          line.shrink_to_fit();
          // send ack
          ack();
          return line;
        }
      } else {
        // invalid checksum.
        return {};
      }
    } else {
      line.push_back(ch);
    }
  }
}

GdbDebugger::GdbDebugger(DebuggerBackend* backend, SOCKET sock)
    : backend_(backend), cpu_(backend->cpu()), remote_(sock) {
  backend->attach();
}

GdbDebugger::~GdbDebugger() { backend_->detach(); }

void GdbDebugger::Run() {
  const auto max_fd = remote_.socket();
  struct timeval ts {};
  while (remote_.is_open()) {
    VLOG(4) << "GdbDebugger::Run: loop";
    // If we had more than 2 here, should move this out of the loop.
    fd_set sock_set;
    FD_ZERO(&sock_set);
    FD_SET(remote_.socket(), &sock_set);
    // Some OSes change this to be the time remaining per call, so reset it each time. ick
    ts.tv_sec = 5;
    ts.tv_usec = 0;
    auto rc = select(max_fd + 1, &sock_set, nullptr, nullptr, &ts);
    if (rc < 0) {
      LOG(ERROR) << "select failed:" << max_fd;
      break;
    } else if (rc == 0) {
      // loop.
      VLOG(3) << "GdbDebugger::Run: select timed out";
      continue;
    }

    // We got one!
    if (FD_ISSET(remote_.socket(), &sock_set)) {
      VLOG(4) << "FD_ISSET: out";
      auto req = remote_.request();
      if (req.empty()) {
        LOG(ERROR) << "empty line. client disconnected?";
        // should never happen since the socket line is set.
        break;
      }
      handle_line(req);
    }
  }
}

bool GdbDebugger::send_ack() {
  LOG(INFO) << "GdbDebugger::send_ack()";
  return remote_.ack();
}

bool GdbDebugger::handle_query_line(const std::string& line) {
  LOG(INFO) << "GdbDebugger::handle_query_line: " << line;
  if (starts_with(line, "qSupported:")) {
    auto optsline = line.substr(11);
    auto opts = wwiv::strings::SplitString(optsline, ";");
    for (const auto& opt : opts) {
      if (opt.back() == '+') {
        LOG(INFO) << "Supports: " << opt;
      }
    }
    // PacketSize=47ff;QPassSignals+;QProgramSignals+;QStartupWithShell+;QEnvironmentHexEncoded+;QEnvironmentReset+;
    // QEnvironmentUnset+;QSetWorkingDir+;QCatchSyscalls+;qXfer:libraries-svr4:read+;augmented-libraries-svr4-read+;
    // qXfer:auxv:read+;qXfer:spu:read+;qXfer:spu:write+;qXfer:siginfo:read+;qXfer:siginfo:write+;
    // qXfer:features:read+;QStartNoAckMode+;qXfer:osdata:read+;
    // multiprocess+;fork-events+;vfork-events+;exec-events+;QNonStop+;QDisableRandomization+;qXfer:threads:read+;
    // ConditionalTracepoints+;TraceStateVaria
    return remote_.response("PacketSize=47ff;swbreak+;hwbreak;xmlRegisters=i8086"); // qXfer:features:read+;
  } else if (starts_with(line, "qTStatus")) {
    return remote_.response("");
  } else if (starts_with(line, "qAttached")) {
    return remote_.response("1"); // attached to existing exe
  } else if (starts_with(line, "qfThreadInfo")) {
    return remote_.response("m1"); // one thread active
  } else if (starts_with(line, "qC")) {
    // current thread id is always 0
    return remote_.response("QC1");
  } 
  LOG(WARNING) << "Unhandled query line: " << line;
  // Send empty response for things we don't know.
  return remote_.response("");
}

bool GdbDebugger::handle_v_line(const std::string& line) {
  LOG(INFO) << "GdbDebugger::handle_v_line";
  if (line == "vMustReplyEmpty") {
    // vMustReplyEmpty gets an empty response
    return remote_.response("");
  }
  remote_.response("");
  return false;
}

bool GdbDebugger::handle_multithread(const std::string& line) {
  LOG(INFO) << "GdbDebugger::handle_multithread";
  if (line.size() < 3) {
    LOG(WARNING) << "Malformed 'H' line: " << line;
    return false;
  }
  switch (line.at(1)) {
  case 'c':
  case 'g': default_thread_id_ = to_number<int>(line.substr(2)); return remote_.response("");
  default: LOG(WARNING) << "Malformed 'H' line: " << line; return false;
  }
}

std::string to_intel_format_hex16(uint16_t d) {
  return fmt::format("{:02X}{:02X}0000", d & 0xFF, (d & 0xff00) >> 8);
}

std::string to_intel_format_hex32(uint16_t d) {
  uint8_t b0 = d & 0x000000ff;
  uint8_t b1 = (d & 0x0000ff00) >> 8;
  uint8_t b2 = (d & 0x00ff0000) >> 16;
  uint8_t b3 = (d & 0xff000000) >> 24;
  return fmt::format("{:02X}{:02X}{:02X}{:02X}", b0, b1, b2, b3);
}

bool GdbDebugger::handle_registers(const std::string& line) {
  LOG(INFO) << "GdbDebugger::handle_registers";
  switch (line.front()) {
  case 'g': {
    auto& r = cpu_->core.regs;
    auto& sr = cpu_->core.sregs;
    std::vector<std::string> regs;
    regs.emplace_back(fmt::format("{:02X}{:02X}0000", r.h.al, r.h.ah));
    regs.emplace_back(fmt::format("{:02X}{:02X}0000", r.h.cl, r.h.ch));
    regs.emplace_back(fmt::format("{:02X}{:02X}0000", r.h.dl, r.h.dh));
    regs.emplace_back(fmt::format("{:02X}{:02X}0000", r.h.bl, r.h.bh));
    regs.emplace_back(fmt::format("{:02X}{:02X}0000", r.x.sp & 0xFF, (r.x.sp & 0xff00) >> 8));
    regs.emplace_back(fmt::format("{:02X}{:02X}0000", r.x.bp & 0xFF, (r.x.bp & 0xff00) >> 8));
    regs.emplace_back(fmt::format("{:02X}{:02X}0000", r.x.si & 0xFF, (r.x.si & 0xff00) >> 8));
    regs.emplace_back(fmt::format("{:02X}{:02X}0000", r.x.di & 0xFF, (r.x.di & 0xff00) >> 8));
    const uint32_t ip = (sr.cs * 0x10) + cpu_->core.ip;
    regs.emplace_back(to_intel_format_hex32(ip));
    regs.emplace_back(to_intel_format_hex16(cpu_->core.flags.value_));
    return remote_.response(JoinStrings(regs, ""));
  }
  case 'p': {
    const auto rn = to_number<int>(line.substr(1));
    LOG(INFO) << "Fetch register #: " << rn;
    return remote_.response("00000000");
  } break;
  }
  return remote_.response("");
}

bool GdbDebugger::handle_stop(const std::string& line) {
  LOG(INFO) << "GdbDebugger::handle_stop";
  // Sorta BS response
  return remote_.response("S01");
}

bool GdbDebugger::handle_line(const std::string& s) {
  switch (s.front()) {
  case 'q': return handle_query_line(s);
  case 'v': return handle_v_line(s);
  case 'H': return handle_multithread(s);
  case 'g': return handle_registers(s);
  case 'p': return handle_registers(s);
  case '?': return handle_stop(s);
  default: LOG(ERROR) << "Unhandled line: " << s; 
    remote_.response("");
    return false;
  }
  return true;
}

} // namespace door86::dbg