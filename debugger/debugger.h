#ifndef INCLUDED_DBG_DEBUGGER_H
#define INCLUDED_DBG_DEBUGGER_H

#include "cpu/x86/cpu.h"
#include "debugger/blocking_queue.h"
#include <mutex>

namespace door86::dbg {


enum class debug_command_id_t {
  cont,
  step,
  step_in,
  step_out,
  list_breakpoints,
  list_watchpoints,
  show_registers,
  show_memory,
};


struct debug_command_t {
  debug_command_id_t cmd;
};

enum class debug_response_id_t { stop, start, terminate };

enum class debugee_state_t { running, stepping, stopped };

struct debug_response_t {
  debug_response_id_t id;
  debugee_state_t state;
  int thread_id{1};
  int exit_code{0};
};

// DebuggerBackend interface
class DebuggerBackend {
public:
  DebuggerBackend(cpu::x86::CPU* cpu);
  ~DebuggerBackend();
  bool add(debug_command_t c);
  bool add(debug_response_t r);
  bool attach();
  bool detach();

  cpu::x86::CPU* cpu() const { return cpu_; }
  debugee_state_t state() const;

  blocking_queue<debug_command_t> cmds_;

  blocking_queue<debug_response_t>& responses() { return responses_; }

private:
  blocking_queue<debug_response_t> responses_;
  void int1(int, door86::cpu::x86::CPU&);
  void int3(int, door86::cpu::x86::CPU&);
  cpu::x86::CPU* cpu_;
  debugee_state_t state_{debugee_state_t ::stepping};
  mutable std::mutex mu_;
};

}


#endif
