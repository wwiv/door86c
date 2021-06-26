#include "cpu/x86/core.h"

#include <iostream>
#include <iomanip>

namespace door86::cpu::x86 {

CPU::CPU() : core(), decoder(), memory(1 << 20) {}

uint8_t& rmreg8(cpu_core& core, uint8_t reg) {
  switch (reg) {
  case 0:
    return core.regs.h.al;
  case 1:
    return core.regs.h.cl;
  case 2:
    return core.regs.h.dl;
  case 3:
    return core.regs.h.bl;
  case 4:
    return core.regs.h.ah;
  case 5:
    return core.regs.h.ch;
  case 6:
    return core.regs.h.dh;
  case 7:
    return core.regs.h.bh;
  }
  // TODO(rushfan): Assert here
  return core.regs.h.bh;
}

uint16_t& rmreg16(cpu_core& core, uint8_t reg) {
  switch (reg) {
  case 0:
    return core.regs.x.ax;
  case 1:
    return core.regs.x.cx;
  case 2:
    return core.regs.x.dx;
  case 3:
    return core.regs.x.bx;
  case 4:
    return core.regs.x.sp;
  case 5:
    return core.regs.x.bp;
  case 6:
    return core.regs.x.si;
  case 7:
    return core.regs.x.di;
  }
  return core.regs.x.di;
}

// Sreg — A segment register.
// The segment register bit assignments are:
// ES = 0, CS = 1, SS = 2, DS = 3, FS = 4, and GS = 5.
uint16_t& rmreg_sreg(cpu_core& core, uint8_t reg) {
  switch (reg) {
  case 0:
    return core.sregs.es;
  case 1:
    return core.sregs.cs;
  case 2:
    return core.sregs.ss;
  case 3:
    return core.sregs.ds;
  case 4:
    return core.sregs.fs;
  case 5:
    return core.sregs.gs;
  default:
    // todo - assert
    return core.sregs.gs;
  }
}

// returns the register for an r8
uint8_t& r8(const instruction_t& inst, cpu_core& core) { 
  return rmreg8(core, inst.mdrm.reg);
}

// returns the register for an r8
uint16_t& r16(const instruction_t& inst, cpu_core& core) { 
  if (inst.metadata.mask & op_mask_reg_is_sreg) {
    return rmreg_sreg(core, inst.mdrm.reg);
  }
  return rmreg16(core, inst.mdrm.reg);
}

static char* ea012[] = {"[BX + SI]", "[BX + DI]", "[BP + SI]", "[BP + DI]",
                        "[SI]",      "[DI]",      "[BP]",      "[BX]"};

uint16_t base_ea(uint8_t rm, const cpu_core& core) {
  switch (rm) {
  case 0:
    return core.regs.x.bx + core.regs.x.si;
  case 1:
    return core.regs.x.bx + core.regs.x.di;
  case 2:
    return core.regs.x.bp + core.regs.x.si;
  case 3:
    return core.regs.x.bp + core.regs.x.di;
  case 4:
    return core.regs.x.si;
  case 5:
    return core.regs.x.di;
  case 6:
    return core.regs.x.bp;
  case 7:
    return core.regs.x.bx;
  }
}

// returns the offset for the effective address described by a modrm byte
// returns the rm portion for the modrm/
// Note this returns a value not a reference
uint16_t& rm16(const instruction_t& inst, cpu_core& core, Memory& mem, uint16_t seg, uint16_t disp) {
  if (inst.mdrm.mod == 0x03) {
    return r16(inst, core);
  }
  if (inst.mdrm.mod == 0 && inst.mdrm.rm == 0x06) {
    return disp;
  }
  uint16_t base = base_ea(inst.mdrm.rm, core);
  if (inst.mdrm.mod == 0) {
    return base;
  } else {
    // TODO: figureout out mem
    //return base + disp;
  }
}

// mostly add
void CPU::execute_0x0(const instruction_t& inst) {
  auto& reg8 = r8(inst, core);
  auto& reg16 = r16(inst, core);
  // TODO(rushfan): Check which segment to use, ds, ss or override by prefix
  uint16_t seg = core.sregs.ds;
  auto& regmem16 = rm16(inst, core, memory, seg, inst.metadata.bits == 8 ? inst.operand8 : inst.operand16);
  switch (inst.op & 0x0f) {
  }
}


bool CPU::execute(uint16_t start_cs, uint16_t start_ip) { 
  core.sregs.cs = start_cs;
  core.ip = start_ip;

  while (running_) {
    int pos = core.sregs.cs + core.ip;
    auto inst = decoder.next_instruction(&memory[pos]);
    core.ip += inst.len;
    const auto family = inst.op >> 4;
    switch (family) { 
      case 0: {
      execute_0x0(inst);
    } break;
    }
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

