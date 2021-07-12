#include "dbg/debugger.h"

#include "core/os.h"
#include <chrono>
#include <string>

namespace door86::dbg {

// TODO(rushfan): Should we move Debugger into LameDebugger and then can use constructor
// and destructor vs. atttach and detach?

Debugger::Debugger(cpu::x86::CPU* cpu) : cpu_(cpu) {
}

Debugger::~Debugger() { 
}


bool Debugger::add(debug_command_t c) { 
  cmds_.push(std::move(c)); 
  return true;
}

bool Debugger::attach() {
  cpu_->int_handlers().try_emplace(
      0x1, std::bind(&Debugger::int1, this, std::placeholders::_1, std::placeholders::_2));
  cpu_->int_handlers().try_emplace(
      0x3, std::bind(&Debugger::int3, this, std::placeholders::_1, std::placeholders::_2));
  cpu_->debugger_attached.store(true);
  return true;
}

bool Debugger::detach() {
  cpu_->int_handlers().erase(0x01);
  cpu_->int_handlers().erase(0x03);
  cpu_->debugger_attached.store(false);
  return true;
}

void Debugger::int1(int, door86::cpu::x86::CPU& cpu) {
  while (!cpu_->debugger_attached.load()) {
    if (cmds_.empty()) {
      wwiv::os::sleep_for(std::chrono::milliseconds(200));
      continue;
    }
    auto cmd = cmds_.pop();
    LOG(INFO) << "has debug command: " << static_cast<int>(cmd.cmd) << std::endl;
    switch (cmd.cmd) { 
    case debug_command_id_t::cont: return;
    case debug_command_id_t::step: return;
    }
  }
}

void Debugger::int3(int, door86::cpu::x86::CPU& cpu) { int1(3, cpu); }


}


