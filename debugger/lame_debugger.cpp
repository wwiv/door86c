#include "debugger/lame_debugger.h"

#include "core/net.h"
#include "core/strings.h"
#include "core/socket_connection.h"

#include <string>
#include <iostream>

#ifdef _WIN32
#else
#include <errno.h>
#include <unistd.h>
#endif

namespace door86::dbg {

LameDebugger::LameDebugger(Debugger* di, SOCKET sock) : di_(di), cpu_(di->cpu()), conn_(sock) {
  di->attach();
}

LameDebugger::~LameDebugger() { di_->detach(); }

void LameDebugger::Run() {
  const auto max_fd = conn_.socket();
  struct timeval ts{};
  while (conn_.is_open()) {
    VLOG(4) << "LameDebugger::Run: loop";
    // If we had more than 2 here, should move this out of the loop.
    fd_set sock_set;
    FD_ZERO(&sock_set);
    FD_SET(conn_.socket(), &sock_set);
    // Some OSes change this to be the time remaining per call, so reset it each time. ick
    ts.tv_sec = 5;
    ts.tv_usec = 0;
    auto rc = select(max_fd + 1, &sock_set, nullptr, nullptr, &ts);
    if (rc < 0) {
      LOG(ERROR) << "select failed:" << max_fd;
      break;
    } else if (rc == 0) {
      // loop.
      VLOG(3) << "LameDebugger::Run: select timed out";
      continue;
    }

    // We got one!
    if (FD_ISSET(conn_.socket(), &sock_set)) {
      VLOG(4) << "FD_ISSET: out";
      auto line = conn_.read_line(1024, std::chrono::seconds(2));
      if (line.empty()) {
        LOG(ERROR) << "empty line. client disconnected?";
        // should never happen since the socket line is set.
        break;
      }
      handle_line(line);
    }

  }
}

void LameDebugger::handle_line(const std::string& line) {
  std::cout << "LameDebugger: " << line << std::endl;
  if (line.empty()) {
    return;
  }
  auto s = wwiv::strings::StringTrim(line);
  auto [cmd, data] = wwiv::strings::SplitOnce(s, " ");
  wwiv::strings::StringTrim(&cmd);
  wwiv::strings::StringLowerCase(&cmd);
  wwiv::strings::StringTrim(&data);
  LOG(INFO) << "LameDebugger::handle_line; cmd: " << cmd << "; data: " << data;
  debug_command_t dc{};
  if (cmd == "step") {
    dc.cmd = debug_command_id_t::step;
    di_->add(dc);
  } else if (cmd == "cont") {
    dc.cmd = debug_command_id_t::cont;
    di_->add(dc);
  } else {
  }
}


} // namespace door86::dbg