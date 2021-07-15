#ifndef INCLUDED_DBG_GDB_DEBUGGER_H
#define INCLUDED_DBG_GDB_DEBUGGER_H

#include "core/net.h"
#include "core/socket_connection.h"
#include "cpu/x86/cpu.h"
#include "debugger/blocking_queue.h"
#include "debugger/debugger.h"

namespace door86::dbg {

// Used for talking the GDB remote serial protocol over a socket.
class GdbRemote {
public:
  GdbRemote(SOCKET sock) : sock_(sock) {}
  ~GdbRemote();

  bool response(const std::string& resp);
  bool ack();
  bool nak();
  // returns true on ack, falss on nak.
  bool wait_ack_or_nak();
  std::string request();

  SOCKET socket() const noexcept { return sock_; };
  void close() { sock_ = INVALID_SOCKET; }
  bool is_open() const noexcept { return sock_ != INVALID_SOCKET; }

private:
  SOCKET sock_;
};

class GdbDebugger {
public:
  GdbDebugger(DebuggerBackend* di, SOCKET sock);
  ~GdbDebugger();

  void Run();
  bool handle_line(const std::string& line);
  bool handle_query_line(const std::string& line);
  bool handle_v_line(const std::string& line);
  bool handle_multithread(const std::string& line);
  bool handle_registers(const std::string& line);
  bool handle_stop(const std::string& line);
  bool handle_mem(const std::string& line);
  bool send_ack();

  bool handle_response(debug_response_t r);

private:
  DebuggerBackend* backend_{nullptr};
  cpu::x86::CPU* cpu_{nullptr};
  GdbRemote remote_;
  int default_thread_id_{0};
};

// validates a GDB ESP line, and returns the line without the
// prefix, suffix bit or checksum, or empty line on error
bool validate_checksum(const std::string& s, const std::string& cs);
} // namespace door86::dbg

#endif