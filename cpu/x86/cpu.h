#ifndef INCLUDED_CPU_X86_CPU_H
#define INCLUDED_CPU_X86_CPU_H

#include "cpu/memory.h"
#include "cpu/x86/cpu_core.h"
#include "cpu/x86/decoder.h"
#include "cpu/x86/regs.h"
#include "cpu/x86/rmm.h"
#include <cstdint>
#include <functional>
#include <map>

// Start with instructons needed for hello world in asm, then expand
// to these, then on to others as needed.
// https://github.com/xem/minix86/blob/gh-pages/src/instructions.js

namespace door86::cpu::x86 {

class CPU {
public:
  CPU();

  // opcode executing
  // TODO(rushfan): Rebucket these into the following
  /** 
    Opcodes in octal; groups/classes:
    * 000-077: arith-logical operations: ADD, ADC,SUB, SBB,AND...
      – 0P[0-7], where P in {0: add, 1: or, 2: adc, 3: sbb, 4: and, 5: sub, 6: xor, 7: cmp}
    * 100-177: INC/PUSH/POP, Jcc,...
    * 200-277: data movement: MOV,LODS,STOS,...
    * 300-377: misc and escape groups  
  */
  
  // set cs:ip then execute
  bool run(uint16_t cs, uint16_t ip);
  // execute using existing cs:ip
  bool run();
  bool execute(const instruction_t& inst);
  void execute_0x0(const instruction_t& inst);
  void execute_0x1(const instruction_t& inst);
  void execute_0x2(const instruction_t& inst);
  void execute_0x3(const instruction_t& inst);
  void execute_0x4(const instruction_t& inst);
  void execute_0x5(const instruction_t& inst);
  void execute_0x6(const instruction_t& inst);
  void execute_0x7(const instruction_t& inst);
  void execute_0x8(const instruction_t& inst);
  void execute_0x9(const instruction_t& inst);
  void execute_0xA(const instruction_t& inst);
  void execute_0xB(const instruction_t& inst);
  void execute_0xC(const instruction_t& inst);
  void execute_0xC0(const instruction_t& inst, int subop);
  void execute_0xC1(const instruction_t& inst, int subop);
  void execute_0xD0(const instruction_t& inst, int subop);
  void execute_0xD1(const instruction_t& inst, int subop);
  void execute_0xD2(const instruction_t& inst, int subop);
  void execute_0xD3(const instruction_t& inst, int subop);
  void execute_0xD(const instruction_t& inst);
  void execute_0xE(const instruction_t& inst);
  void execute_0xF6(const instruction_t& inst, int subop);
  void execute_0xF7(const instruction_t& inst, int subop);
  void execute_0xFE(const instruction_t& inst, int subop);
  void execute_0xFF(const instruction_t& inst, int subop);
  void execute_0xF(const instruction_t& inst);

  // handle rep and repne
  void rep(const instruction_t& inst);

  // These are useful when handling rep and repne
  // individual instructuctions not handled in execute_0xX loop.
  void scas_m8(const instruction_t& inst);
  void scas_m16(const instruction_t& inst);

  // stack handling

  void push(uint16_t val);
  uint16_t pop();

  // interrupt handling

  void call_interrupt(int num);

  // Processor State

  // TODO: add in pause and resume separately to handlle HLT instruction
  void halt() { running_ = false; }
  void resume() { running_ = true; }

  // flags

  // flags for OF, SF, ZF, AF, PF CF for 8 bit values
  void parity_szp8(uint8_t oval, uint8_t nval);
  void parity_szp16(uint16_t oval, uint16_t nval);

  // Interrupts
  std::map<int, std::function<void(int num, CPU& cpu)>>& int_handlers() { return int_handlers_; }

  // Public structures

  cpu_core core;
  Decoder decoder;
  Memory memory;


  // Helpers
private:
  Rmm<RmmType::REGISTER, uint8_t> r8(const instruction_t& inst);
  Rmm<RmmType::REGISTER, uint16_t> r16(const instruction_t& inst);
  Rmm<RmmType::REGISTER, uint16_t> r16(int regnum);
  Rmm<RmmType::REGISTER, uint8_t> r8(uint8_t* reg);
  Rmm<RmmType::REGISTER, uint16_t> r16(uint16_t* reg);
  Rmm<RmmType::EITHER, uint8_t> rmm8(const instruction_t& inst);
  Rmm<RmmType::EITHER, uint16_t> rmm16(const instruction_t& inst);

  bool running_{true};
  // default interrupt handlers.  default means it's not been overridden
  // by DOS code.
  std::map<int, std::function<void(int num, CPU& cpu)>> int_handlers_;
};


}

#endif