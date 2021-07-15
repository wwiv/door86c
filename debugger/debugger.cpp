#include "debugger/debugger.h"

#include "core/os.h"
#include <chrono>
#include <string>

namespace door86::dbg {

// TODO(rushfan): Should we move DebuggerBackend into LameDebugger and then can use constructor
// and destructor vs. atttach and detach?

DebuggerBackend::DebuggerBackend(cpu::x86::CPU* cpu) : cpu_(cpu) {
}

DebuggerBackend::~DebuggerBackend() { 
}

bool DebuggerBackend::add(debug_command_t c) { 
  cmds_.push(std::move(c)); 
  return true;
}

bool DebuggerBackend::add(debug_response_t c) {
  responses_.push(std::move(c));
  return true;
}

bool DebuggerBackend::attach() {
  cpu_->int_handlers().try_emplace(
      0x1, std::bind(&DebuggerBackend::int1, this, std::placeholders::_1, std::placeholders::_2));
  cpu_->int_handlers().try_emplace(
      0x3, std::bind(&DebuggerBackend::int3, this, std::placeholders::_1, std::placeholders::_2));
  cpu_->debugger_attached.store(true);
  return true;
}

bool DebuggerBackend::detach() {
  cpu_->int_handlers().erase(0x01);
  cpu_->int_handlers().erase(0x03);
  cpu_->debugger_attached.store(false);
  return true;
}

debugee_state_t DebuggerBackend::state() const {
  std::lock_guard<std::mutex> lock(mu_);
  return state_;
}

void DebuggerBackend::int1(int, door86::cpu::x86::CPU& cpu) {
  while (cpu_->debugger_attached.load()) {
    if (cmds_.empty()) {
      {
        std::lock_guard<std::mutex> lock(mu_);
        if (state_ == debugee_state_t::running) {
          // TODO(rushfan): Look for breakpoints here and stop if we find one.
          // If we're running, don't stop
          return;
        }
        // we're stopped at the moment.
        state_ = debugee_state_t::stopped;
      }
      wwiv::os::sleep_for(std::chrono::milliseconds(200));
      continue;
    }
    auto cmd = cmds_.pop();
    LOG(INFO) << "has debug command: " << static_cast<int>(cmd.cmd) << std::endl;

    std::lock_guard<std::mutex> lock(mu_);
    switch (cmd.cmd) { 
    case debug_command_id_t::cont: state_ = debugee_state_t::running; return;
    case debug_command_id_t::step: state_ = debugee_state_t::stepping; return;
    }
  }
}

void DebuggerBackend::int3(int, door86::cpu::x86::CPU& cpu) { int1(3, cpu); }


}


