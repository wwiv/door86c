#include "cpu/x86/core.h"

#include <iostream>
#include <iomanip>

namespace door86::cpu::x86 {

CPU::CPU() : core(), decoder(), memory(1 << 20) {}

// returns the register for an r16
uint16_t& r16(const instruction_t& inst, cpu_core& core) { 
  if (inst.metadata.mask & op_mask_reg_is_sreg) {
    return core.sregs.regref(inst.mdrm.reg);
  }
  return core.regs.x.regref(inst.mdrm.reg);
}

// "[BX + SI]", "[BX + DI]", "[BP + SI]", "[BP + DI]",
// "[SI]",      "[DI]",      "[BP]",      "[BX]"

// base without any displacement.
uint16_t base_ea(uint8_t rm, const cpu_core& core) {
  switch (rm) {
  case 0: return core.regs.x.bx + core.regs.x.si;
  case 1: return core.regs.x.bx + core.regs.x.di;
  case 2: return core.regs.x.bp + core.regs.x.si;
  case 3: return core.regs.x.bp + core.regs.x.di;
  case 4: return core.regs.x.si;
  case 5: return core.regs.x.di;
  case 6: return core.regs.x.bp;
  case 7: return core.regs.x.bx;
  }
  // Todo(CHECK FAIL HERE?) gpf?
  return 0;
}

// **** NOTE: Keep rm8 and rm16 in sync

// returns the offset for the effective address described by a modrm byte
// returns the rm portion for the modrm
uint8_t& rm8(const instruction_t& inst, cpu_core& core, Memory& mem, uint16_t seg, uint16_t disp) {
  if (inst.mdrm.mod == 0x03) {
    // mod 3 returns a reference to a CPU register
    return core.regs.h.regref(inst.mdrm.reg);
  }
  if (inst.mdrm.mod == 0 && inst.mdrm.rm == 0x06) {
    return mem.byteref(seg, disp);
  }
  uint16_t base = base_ea(inst.mdrm.rm, core);
  if (inst.mdrm.mod == 0) {
    return mem.byteref(seg, base);
  } else {
    // mod 1 and 2 adds a 8 or 16 bit dispacemnt
    return mem.byteref(seg, base + disp);
  }
}

// returns the offset for the effective address described by a modrm byte
// returns the rm portion for the modrm
uint16_t& rm16(const instruction_t& inst, cpu_core& core, Memory& mem, uint16_t seg, uint16_t disp) {
  if (inst.mdrm.mod == 0x03) {
    // mod 3 returns a reference to a CPU register
    return r16(inst, core);
  }
  if (inst.mdrm.mod == 0 && inst.mdrm.rm == 0x06) {
    return mem.wordref(seg, disp);
  }
  const auto base = base_ea(inst.mdrm.rm, core);
  if (inst.mdrm.mod == 0) {
    return mem.wordref(seg, base);
  } else {
    // mod 1 and 2 adds a 8 or 16 bit dispacemnt
    return mem.wordref(seg, base + disp);
  }
}

// mostly add
void CPU::execute_0x0(const instruction_t& inst) {
  auto& reg8 = core.regs.h.regref(inst.mdrm.reg);
  auto& reg16 = r16(inst, core);
  // TODO(rushfan): Check which segment to use, ds, ss or override by prefix
  uint16_t seg = core.sregs.ds;
  auto& regmem16 = rm16(inst, core, memory, seg, inst.metadata.bits == 8 ? inst.operand8 : inst.operand16);
  switch (inst.op & 0x0f) {
  case 0x0: regmem16 += reg8; break;
  case 0x1: regmem16 += reg16; break;
  case 0x2: {
    auto& regmem8 =
        rm8(inst, core, memory, seg, inst.metadata.bits == 8 ? inst.operand8 : inst.operand16);
    reg8 += regmem8;
  } break;
  case 0x3: reg16 += regmem16; break;
  case 0x4: core.regs.h.al += inst.operand8; break;
  case 0x5: core.regs.x.ax += inst.operand16; break;
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

