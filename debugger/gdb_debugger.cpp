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

bool GdbDebugger::handle_response(debug_response_t r) {
  switch (r.id) {
  case debug_response_id_t::stop: {
    LOG(INFO) << "Handling response: stop";
    return remote_.response(fmt::format("S{:02X}", r.thread_id));
  } break;
  case debug_response_id_t::terminate: {
    LOG(INFO) << "Handling response: terminate";
    return remote_.response(fmt::format("W{:02X}", r.exit_code));
  } break;
  default: LOG(INFO) << "Handling UNKNOWN response: "; return false;
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
    ts.tv_sec = 1;
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

    while (!backend_->responses().empty()) {
      handle_response(backend_->responses().pop());
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
  } else if (starts_with(line, "qfThreadInfo")) { // qsThreadInfo
    return remote_.response("m1");                // one thread active
  } else if (starts_with(line, "qsThreadInfo")) {
    return remote_.response("l");
  } else if (starts_with(line, "qC")) {
    // current thread id is always 0
    return remote_.response("QC1");
  } 
  LOG(WARNING) << "Unhandled query line: " << line;
  // Send empty response for things we don't know.
  return remote_.response("");
}

bool GdbDebugger::handle_v_line(const std::string& line) {
  LOG(INFO) << "GdbDebugger::handle_v_line: " << line;
  if (line == "vMustReplyEmpty") {
    // vMustReplyEmpty gets an empty response
    return remote_.response("");
  } else if (line == "vCont?") {
    return remote_.response("vCont;c;C;r;s;S;t");
  } else if (starts_with(line, "vCont;")) {
    auto vrawdata = line.substr(6);
    auto actions = SplitString(vrawdata, ";");
    // hack:
    char cmd = vrawdata.empty() ? 'X' : vrawdata.front();
    if (cmd == 'S' || cmd == 's') {
      backend_->add(debug_command_t{debug_command_id_t::step});
      // TODO(rushfan): Find way to send stop packet later
    } else if (cmd == 'C' || cmd == 'c') {
      // TODO(rushfan): Find way to send stop packet later
      backend_->add(debug_command_t{debug_command_id_t::cont});
    }
  }
  // need to get some way to send stop packets from int1. This is a hack
  for (int i = 0; i < 10 && backend_->state() != debugee_state_t::stopped; i++) {
    wwiv::os::sleep_for(std::chrono::microseconds(100));
  }
  if (backend_->state() == debugee_state_t::stopped) {
    remote_.response("S05");
  }
  return true;
}

bool GdbDebugger::handle_multithread(const std::string& line) {
  LOG(INFO) << "GdbDebugger::handle_multithread";
  if (line.size() < 3) {
    LOG(WARNING) << "Malformed 'H' line: " << line;
    return false;
  }
  switch (line.at(1)) {
  case 'c':
  case 'g': default_thread_id_ = to_number<int>(line.substr(2), 16); return remote_.response("");
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

std::string to_intel_format_seg_off(uint16_t seg, uint16_t off) {
  const uint32_t d = (seg * 0x10) + off;
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
    regs.emplace_back(to_intel_format_hex16(r.x.ax));
    regs.emplace_back(to_intel_format_hex16(r.x.cx));
    regs.emplace_back(to_intel_format_hex16(r.x.dx));
    regs.emplace_back(to_intel_format_hex16(r.x.bx));
    regs.emplace_back(to_intel_format_hex16(r.x.sp));
    regs.emplace_back(to_intel_format_hex16(r.x.bp));
    regs.emplace_back(to_intel_format_hex16(r.x.si));
    regs.emplace_back(to_intel_format_hex16(r.x.di));
    regs.emplace_back(to_intel_format_seg_off(sr.cs, cpu_->core.ip));
    regs.emplace_back(to_intel_format_hex16(cpu_->core.flags.value_));
    regs.emplace_back(to_intel_format_seg_off(sr.cs, 0));
    regs.emplace_back(to_intel_format_seg_off(sr.ss, 0));
    regs.emplace_back(to_intel_format_seg_off(sr.ds, 0));
    regs.emplace_back(to_intel_format_seg_off(sr.es, 0));
    regs.emplace_back(to_intel_format_seg_off(sr.fs, 0));
    regs.emplace_back(to_intel_format_seg_off(sr.gs, 0));
    return remote_.response(JoinStrings(regs, ""));
  }
  case 'p': {
    const auto rn = to_number<int>(line.substr(1), 16);
    LOG(INFO) << "Fetch register #: " << rn;
    return remote_.response("00000000");
  } break;
  }
  return remote_.response("");
}

bool GdbDebugger::handle_stop(const std::string& line) {
  LOG(INFO) << "GdbDebugger::handle_stop";
  // Sorta BS response
  return remote_.response("S05");
}

bool GdbDebugger::handle_mem(const std::string& line) {
  LOG(INFO) << "GdbDebugger::handle_stop";
  const auto v = SplitString(line.substr(1), ",");
  if (v.size() < 2) {
    LOG(ERROR) << "Malformed m packet: " << line;
    // BS response
    return remote_.response("");
  }
  auto addr = to_number<uint32_t>(v.at(0), 16);
  auto len = to_number<int>(v.at(1), 16);

  std::string resp;
  resp.reserve(len * 4);
  for (int i = 0; i < len; i++) {
    resp.append(fmt::format("{:02X}", cpu_->memory.abs8(addr + i)));
  }

  return remote_.response(resp);
}

bool GdbDebugger::handle_line(const std::string& s) {
  switch (s.front()) {
  case 'q': return handle_query_line(s);
  case 'v': return handle_v_line(s);
  case 'H': return handle_multithread(s);
  case 'g': return handle_registers(s);
  case 'p': return handle_registers(s);
  case '?': return handle_stop(s);
  case 'm': return handle_mem(s);
  default: LOG(ERROR) << "Unhandled line: " << s; 
    remote_.response("");
    return false;
  }
  return true;
}

} // namespace door86::dbg