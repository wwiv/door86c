#ifndef INCLUDED_DBG_LAME_DEBUGGER_H
#define INCLUDED_DBG_LAME_DEBUGGER_H

#include "core/net.h"
#include "core/socket_connection.h"
#include "cpu/x86/cpu.h"
#include "debugger/debugger.h"
#include "debugger/blocking_queue.h"

namespace door86::dbg {

class LameDebugger {
public:
  LameDebugger(DebuggerBackend* di, SOCKET sock);
  ~LameDebugger();

  void Run();
  void handle_line(const std::string& line);

private:
  DebuggerBackend* backend_{nullptr};
  cpu::x86::CPU* cpu_{nullptr};
  wwiv::core::SocketConnection conn_;
};

} // namespace door86::dbg

#endif