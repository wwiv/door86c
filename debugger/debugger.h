#ifndef INCLUDED_DBG_DEBUGGER_H
#define INCLUDED_DBG_DEBUGGER_H

#include "cpu/x86/cpu.h"
#include "debugger/blocking_queue.h"

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

// DebuggerBackend interface
class DebuggerBackend {
public:
  DebuggerBackend(cpu::x86::CPU* cpu);
  ~DebuggerBackend();
  bool add(debug_command_t c);
  bool attach();
  bool detach();

  cpu::x86::CPU* cpu() const { return cpu_; }

  blocking_queue<debug_command_t> cmds_;

private:
  void int1(int, door86::cpu::x86::CPU&);
  void int3(int, door86::cpu::x86::CPU&);
  cpu::x86::CPU* cpu_;
};

}


#endif
