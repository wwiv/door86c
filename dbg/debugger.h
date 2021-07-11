#ifndef INCLUDED_DBG_H
#define INCLUDED_DBG_H

#include "dbg/blocking_queue.h"

namespace door86::dbg {

enum class debug_command_t {
  cont,
  step,
  step_in,
  step_out,
  list_breakpoints,
  list_watchpoints,
  show_registers,
  show_memory,
};

struct debug_command_info {
  debug_command_t cmd;
};

// Debugger interface
class Debugger {
public:
  Debugger(CPU* cpu);
  bool add(debug_command_info& c);

  blocking_queue<debug_command> cmds_;

private:
  CPU* cpu_;
};

}


#endif
