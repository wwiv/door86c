#include "cpu/x86/core.h"

#include <iostream>
#include <iomanip>

namespace door86::cpu::x86 {

CPU::CPU() : core(), decoder(), memory(1 << 20) {}


bool CPU::execute(uint16_t start_cs, uint16_t start_ip) { 
  core.sregs.cs = start_cs;
  core.ip = start_ip;

  while (running_) {
    int pos = core.sregs.cs + core.ip;
    auto inst = decoder.next_instruction(&memory[pos]);
    core.ip += inst.len;

    std::cout << "Instruction: " << inst.metadata.name << std::endl;
    // hack
    if (inst.op == 0xcd) {
      // stop on interrupt
      running_ = false;
    }
  }
  return true;
}


} // namespace door86::cpu::x86

