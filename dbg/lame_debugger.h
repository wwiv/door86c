#ifndef INCLUDED_DBG_LAME_DEBUGGER_H
#define INCLUDED_DBG_LAME_DEBUGGER_H

#include "core/net.h"
#include "core/socket_connection.h"
#include "cpu/x86/cpu.h"
#include "dbg/debugger.h"
#include "dbg/blocking_queue.h"

namespace door86::dbg {

class LameDebugger {
public:
  LameDebugger(Debugger* di, SOCKET sock);
  ~LameDebugger();

  void Run();
  void handle_line(const std::string& line);

private:
  Debugger* di_{nullptr};
  cpu::x86::CPU* cpu_{nullptr};
  wwiv::core::SocketConnection conn_;
};

} // namespace door86::dbg

#endif