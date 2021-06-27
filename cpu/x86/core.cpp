#include "cpu/x86/core.h"

#include "fmt/format.h"
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

// Returns the default segment to use for an rm value of 0-7
// per the Intel docs: The default segment register is SS for the effective addresses 
// containing a BP index, DS for other effective addresses.
// check modd too (for modrm) since mod of 0 means there is no BP for #6
segment_t default_segment_for_index(uint8_t mod, uint8_t rm) { 
  if (rm == 2 || rm == 3 || (rm == 6 && mod != 0)) {
    return segment_t::SS;
  }
  return segment_t::DS;
}
    // base without any displacement.
uint16_t base_ea(reg_mod_rm modrm, const cpu_core& core) {
  auto rm = modrm.rm;
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

// Returns the effective address offset (not segment) from the modrm byte
// and following offset.
uint16_t effective_address(const instruction_t& inst, const cpu_core& core) {
  if (inst.mdrm.mod == 0) {
    const auto disp = inst.metadata.bits == 8 ? inst.operand8 : inst.operand16;
    return disp;
  } else if (inst.mdrm.mod == 3) {
    // TODO(rushfan): Should we fail here?
    std::cerr << "Whoops! Getting Effective Address for mod3??!?!?" << std::endl;
    return 0;
  } else {
    // mod 1 and 2 adds a 8 or 16 bit dispacemnt
    const auto base = base_ea(inst.mdrm, core);
    const auto disp = inst.metadata.bits == 8 ? inst.operand8 : inst.operand16;
    return base + disp;
  }
}

// **** NOTE: Keep rm8 and rm16 in sync

// returns the offset for the effective address described by a modrm byte
// returns the rm portion for the modrm
uint8_t& rm8(const instruction_t& inst, cpu_core& core, Memory& mem) {
  // Pick the overridden segment, or just the default one for the instruction.
  const segment_t senum =
      inst.seg_override.value_or(default_segment_for_index(inst.mdrm.mod, inst.mdrm.rm));
  uint16_t seg = core.sregs.regref(senum);
  if (inst.mdrm.mod == 0x03) {
    // mod 3 returns a reference to a CPU register
    return core.regs.h.regref(inst.mdrm.rm);
  }
  if (inst.mdrm.mod == 0 && inst.mdrm.rm == 0x06) {
    const auto disp = inst.metadata.bits == 8 ? inst.operand8 : inst.operand16;
    return mem.byteref(seg, disp);
  }
  const auto offset = effective_address(inst, core);
  return mem.byteref(seg, offset);
}

// returns the offset for the effective address described by a modrm byte
// returns the rm portion for the modrm
uint16_t& rm16(const instruction_t& inst, cpu_core& core, Memory& mem) {
  const segment_t senum =
      inst.seg_override.value_or(default_segment_for_index(inst.mdrm.mod, inst.mdrm.rm));
  uint16_t seg = core.sregs.regref(senum);
  if (inst.mdrm.mod == 0x03) {
    // mod 3 returns a reference to a CPU register.
    // don't ue r16 since that applies segment override, and ModR/M bytes
    // in mod 3 don't ever use segment registers.
    return core.regs.x.regref(inst.mdrm.rm);
  }
  if (inst.mdrm.mod == 0 && inst.mdrm.rm == 0x06) {
    const auto disp = inst.metadata.bits == 8 ? inst.operand8 : inst.operand16;
    return mem.wordref(seg, disp);
  }
  const auto offset = effective_address(inst, core);
  return mem.wordref(seg, offset);
}

// TODO(rushfan): Make generic way to set flags after operastions
// mostly add
void CPU::execute_0x0(const instruction_t& inst) {
  auto& reg16 = r16(inst, core);
  auto& regmem16 = rm16(inst, core, memory);
  switch (inst.op & 0x0f) {
  // "00":"add r/m8, r8",
  case 0x0: {
    auto& reg8 = core.regs.h.regref(inst.mdrm.reg);
    regmem16 += reg8;
  } break;
  // "01" : "add r/m16/32, r16/32",
  case 0x1: regmem16 += reg16; break;
  // 02/r":"add r8, r/m8
  case 0x2: {
    auto& reg8 = core.regs.h.regref(inst.mdrm.reg);
    const auto& regmem8 = rm8(inst, core, memory);
    reg8 += regmem8;
  } break;
  // 03":"add r16/32, r/m16/32
  case 0x3: reg16 += regmem16; break;
  // 04":"add al, imm8
  case 0x4: core.regs.h.al += inst.operand8; break;
  // 05":"add ax, imm16/32
  case 0x5: core.regs.x.ax += inst.operand16; break;
  // 06 PUSH ES
  case 0x6: push(core.sregs.es); break;
  // 07 POP ES
  case 0x7: core.sregs.es = pop(); break;
  }
}

void CPU::execute_0x8(const instruction_t& inst) {
  switch (inst.op & 0x0f) {
  // "8A/r":"mov r8, r/m8",
  case 0xA: {
    auto& reg8 = core.regs.h.regref(inst.mdrm.reg);
    auto& regmem8 = rm8(inst, core, memory);
    reg8 = regmem8;
  } break;
  // "8D":"lea r16/32, m",
  case 0xD: {
    auto& reg16 = r16(inst, core);
    reg16 = effective_address(inst, core);
  } break;
  // "8E/r":"mov Sreg, r/m16",
  case 0xE: {
    auto& regmem16 = rm16(inst, core, memory);
    // actually segment per metadata
    auto& reg16 = r16(inst, core);
    reg16 = regmem16;
  } break;
  }
}

void CPU::execute_0xB(const instruction_t& inst) {
  const auto wide = inst.op & 0x08;
  const auto regnum = inst.op & 0x07;
  if (wide) {
    auto& reg = core.regs.x.regref(regnum);
    reg = inst.operand16;
  } else {
    auto& reg = core.regs.h.regref(regnum);
    reg = inst.operand8;
  }
}

void CPU::execute_0xC(const instruction_t& inst) {
  switch (inst.op & 0x0f) {
  // "CD":"int imm8",
  case 0xC: call_interrupt(0x03); break;
  case 0xD: call_interrupt(inst.operand8); break;
  }
}

void CPU::call_interrupt(int num) { 
  if (num == 0x21) {
    fmt::print("DOS Interrupt: 0x{:04x}\r\n", core.regs.x.ax);
    switch (core.regs.h.ah) {
    // display string
    case 0x09: {
      for (auto offset = core.regs.x.dx;; ++offset) {
        seg_address_t addr{core.sregs.ds, offset};
        const auto& m = memory.byteref(addr);
        if (m == '$') {
          break;
        }
        if (m > 128) {
          break;
        }
        fputc(m, stdout);
      }
    } break;
    case 0x4c: // terminate app.
      running_ = false;
      break;
    }
    if (core.regs.h.ah == 0x4c) {
    }
  } else {
    fmt::print("Interrupt Num: 0x{:02x}\r\n", num);
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
    case 0x0: {
      execute_0x0(inst);
    } break;
    case 0x8: {
      execute_0x8(inst);
    } break;
    case 0xB: {
      execute_0xB(inst);
    } break;
    case 0xC: {
      execute_0xC(inst);
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


void CPU::push(uint16_t val) { 
  // To push we decrement the stack pointer and then add it.
  core.regs.x.sp -= 2;
  // TODO(rushfan): assert if sp <= 0x0000
  auto& m = memory.wordref(core.sregs.ss, core.regs.x.sp);
  m = val;
}

uint16_t CPU::pop() {
  const auto m = memory.wordref(core.sregs.ss, core.regs.x.sp);
  core.regs.x.sp += 2; 
  // TODO(rushfan): assert if sp >= 0xffff
  return m;
}

} // namespace door86::cpu::x86

