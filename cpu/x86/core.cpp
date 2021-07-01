#include "cpu/x86/core.h"

#include "core/log.h"
#include "fmt/format.h"
#include <iostream>
#include <iomanip>

namespace door86::cpu::x86 {

CPU::CPU() : core(), decoder(), memory(1 << 20) {}

// returns the register for an r16
static uint16_t r16(const instruction_t& inst, cpu_core& core) { 
  if (inst.metadata.mask & op_mask_reg_is_sreg) {
    return core.sregs.get(inst.mdrm.reg);
  }
  return core.regs.x.get(inst.mdrm.reg);
}

// sets an r16 value (either reg or sreg depending on the instruction)
static void r16(const instruction_t& inst, cpu_core& core, uint16_t value) { 
  if (inst.metadata.mask & op_mask_reg_is_sreg) {
    core.sregs.set(inst.mdrm.reg, value);
  }
  core.regs.x.set(inst.mdrm.reg, value);
}

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
static uint16_t effective_address(const instruction_t& inst, const cpu_core& core) {
  if (inst.mdrm.mod == 0) {
    const auto disp = inst.metadata.bits == 8 ? inst.operand8 : inst.operand16;
    return disp;
  } else if (inst.mdrm.mod == 3) {
    // TODO(rushfan): Should we fail here?
    LOG(WARNING) << "Whoops! Getting Effective Address for mod3??!?!?";
    return 0;
  } else {
    // mod 1 and 2 adds a 8 or 16 bit dispacemnt
    const auto base = base_ea(inst.mdrm, core);
    const auto disp = inst.metadata.bits == 8 ? inst.operand8 : inst.operand16;
    return base + disp;
  }
}

// **** NOTE: Keep rmm8 and rmm16 in sync

// returns the offset for the effective address described by a modrm byte
// returns the rm portion for the modrm
Rmm<uint8_t> rmm8(const instruction_t& inst, cpu_core& core, Memory& mem) {
  // Pick the overridden segment, or just the default one for the instruction.
  const auto seg_index =
      inst.seg_override.value_or(default_segment_for_index(inst.mdrm.mod, inst.mdrm.rm));
  const uint16_t seg = core.sregs.get(seg_index);
  if (inst.mdrm.mod == 0x03) {
    // mod 3 returns a reference to a CPU register
    return Rmm<uint8_t>(core.regs.h.regptr(inst.mdrm.rm));
  }
  if (inst.mdrm.mod == 0 && inst.mdrm.rm == 0x06) {
    const auto offset = inst.metadata.bits == 8 ? inst.operand8 : static_cast<uint8_t>(inst.operand16 & 0xff);
    return Rmm<uint8_t>(&mem, seg, offset);
  }
  const auto offset = effective_address(inst, core);
  return Rmm<uint8_t>(&mem, seg, offset);
}

Rmm<uint16_t> rmm16(const instruction_t& inst, cpu_core& core, Memory& mem) {
  const segment_t senum =
      inst.seg_override.value_or(default_segment_for_index(inst.mdrm.mod, inst.mdrm.rm));
  uint16_t seg = core.sregs.get(senum);
  if (inst.mdrm.mod == 0x03) {
    // mod 3 returns a reference to a CPU register.
    // don't ue r16 since that applies segment override, and ModR/M bytes
    // in mod 3 don't ever use segment registers.
    return Rmm<uint16_t>(core.regs.x.regptr(inst.mdrm.rm));
  }
  if (inst.mdrm.mod == 0 && inst.mdrm.rm == 0x06) {
    const auto offset = inst.metadata.bits == 8 ? inst.operand8 : inst.operand16;
    return Rmm<uint16_t>(&mem, seg, offset);
  }
  const auto offset = effective_address(inst, core);
  return Rmm<uint16_t>(&mem, seg, offset);
}

// TODO(rushfan): Make generic way to set flags after operations
// mostly add
void CPU::execute_0x0(const instruction_t& inst) {
  auto regmem16 = rmm16(inst, core, memory);
  switch (inst.op & 0x0f) {
  // "00":"add r/m8, r8",
  case 0x0: {
    const auto reg8 = core.regs.h.get(inst.mdrm.reg);
    regmem16 += reg8;
  } break;
  // "01" : "add r/m16/32, r16/32",
  case 0x1: regmem16 += r16(inst, core); break;
  // 02/r":"add r8, r/m8
  case 0x2: {
    const auto reg8 = core.regs.h.get(inst.mdrm.reg);
    const auto regmem8 = rmm8(inst, core, memory);
    core.regs.h.set(inst.mdrm.reg, regmem8.get() + reg8);
  } break;
  // 03":"add r16/32, r/m16/32
  case 0x3: {
    const auto reg16 = r16(inst, core);
    r16(inst, core, reg16 + regmem16.get());
  } break;
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

void CPU::execute_0x1(const instruction_t& inst) {
  switch (inst.op & 0x0f) {}
}
void CPU::execute_0x2(const instruction_t& inst) {
  switch (inst.op & 0x0f) {}
}

void CPU::execute_0x3(const instruction_t& inst) {
  switch (inst.op & 0x0f) {
  case 0x0: {
    auto r = core.regs.h.get(inst.mdrm.reg);
    auto rm = rmm8(inst, core, memory);
    const auto val = rm.get() ^ r;
    rm.set(val);
  } break;
  case 0x1: {
    auto r = core.regs.x.get(inst.mdrm.reg);
    auto rm = rmm16(inst, core, memory);
    const auto val = rm.get() ^ r;
    rm.set(val);
  } break;
  case 0x2: {
    auto r = core.regs.h.get(inst.mdrm.reg);
    auto rm = rmm8(inst, core, memory);
    core.regs.h.set(inst.mdrm.reg, r ^ rm.get());
  } break;
  case 0x3: {
    auto r = core.regs.x.get(inst.mdrm.reg);
    auto rm = rmm16(inst, core, memory);
    core.regs.x.set(inst.mdrm.reg, r ^ rm.get());
  } break;
  case 0x4: {
    core.regs.h.al ^= inst.operand8;
  } break;
  case 0x5: {
    core.regs.x.ax ^= inst.operand16;
  } break;
  }
}

void CPU::execute_0x4(const instruction_t& inst) {
  switch (inst.op & 0x0f) {}
}

void CPU::execute_0x5(const instruction_t& inst) {
  switch (inst.op & 0x0f) {}
}

void CPU::execute_0x6(const instruction_t& inst) {
  switch (inst.op & 0x0f) {}
}

void CPU::execute_0x7(const instruction_t& inst) {
  switch (inst.op & 0x0f) {}
}

void CPU::execute_0x8(const instruction_t& inst) {
  switch (inst.op & 0x0f) {
  // 83/0":"add r/m16/32, imm8
  case 0x3: {
    // only add if reg == 0
    const auto& reg = inst.mdrm.reg;
    switch (reg) {
    case 0x0: {
      auto regmem16 = rmm16(inst, core, memory);
      regmem16 += inst.operand8;
    } break;
    default: {
      VLOG(1) << "Unhandled submode of 0x83: " << reg;
      auto regmem16 = rmm16(inst, core, memory);
      regmem16 += inst.operand8;
    } break;
    }
  } break;
  // "8A/r":"mov r8, r/m8",
  case 0xA: {
    auto regmem8 = rmm8(inst, core, memory);
    core.regs.h.set(inst.mdrm.reg, regmem8.get());
  } break;
  // "8D":"lea r16/32, m",
  case 0xD: {
    r16(inst, core, effective_address(inst, core));
  } break;
  // "8E/r":"mov Sreg, r/m16",
  case 0xE: {
    auto regmem16 = rmm16(inst, core, memory);
    // actually segment per metadata
    r16(inst, core, regmem16.get());
  } break;
  }
}

void CPU::execute_0x9(const instruction_t& inst) {
  switch (inst.op & 0x0f) {}
}

void CPU::execute_0xA(const instruction_t& inst) {
  switch (inst.op & 0x0f) {
  case 0x0: {
    const auto seg_index =
        inst.seg_override.value_or(default_segment_for_index(inst.mdrm.mod, inst.mdrm.rm));
    const uint16_t seg = core.sregs.get(seg_index);
    // offset is inst.operand16
    core.regs.h.al = memory.get<uint8_t>(seg, inst.operand8);
  } break;
  case 0x1: {
    const auto seg_index =
        inst.seg_override.value_or(default_segment_for_index(inst.mdrm.mod, inst.mdrm.rm));
    const uint16_t seg = core.sregs.get(seg_index);
    // offset is inst.operand16
    core.regs.x.ax = memory.get<uint16_t>(seg, inst.operand16);
  } break;
  case 0x2: {
    const auto seg_index =
        inst.seg_override.value_or(default_segment_for_index(inst.mdrm.mod, inst.mdrm.rm));
    const uint16_t seg = core.sregs.get(seg_index);
    // offset is inst.operand16
    memory.set<uint8_t>(seg, inst.operand8, core.regs.h.al);
  } break;
  //  mov [seg:off],ax
  case 0x3: {
    const auto seg_index =
        inst.seg_override.value_or(default_segment_for_index(inst.mdrm.mod, inst.mdrm.rm));
    const uint16_t seg = core.sregs.get(seg_index);
    // offset is inst.operand16
    memory.set<uint16_t>(seg, inst.operand16, core.regs.x.ax);
  } break;
  }
}

void CPU::execute_0xB(const instruction_t& inst) {
  const auto wide = inst.op & 0x08;
  const auto regnum = inst.op & 0x07;
  if (wide) {
    core.regs.x.set(regnum, inst.operand16);
  } else {
    core.regs.h.set(regnum, inst.operand8);
  }
}

void CPU::execute_0xC(const instruction_t& inst) {
  switch (inst.op & 0x0f) {
  // RET (near call and clear some bytes of stack)
  case 0x2: {
    core.ip = pop();
    core.regs.x.sp += inst.operand16;
  } break;
  // RET (near call)
  case 0x3: {
    core.ip = pop();
  } break;
  // LES r16,m16:16 - Load ES:r16 with far pointer from memory.
  case 0x4: {
    const segment_t senum =
        inst.seg_override.value_or(default_segment_for_index(inst.mdrm.mod, inst.mdrm.rm));
    const uint16_t seg = core.sregs.get(senum);
    core.regs.x.set(inst.mdrm.reg, memory.get<uint16_t>(seg, inst.operand16));
    core.sregs.es = inst.mdrm.reg, memory.get<uint16_t>(seg, inst.operand16 + 2);
  } break;
  // LDS r16,m16:16 - Load DS:r16 with far pointer from memory.
  // 
  // For example, the instruction LDS SI, [200h] is equivalent to the pair of
  // instructions:
  // MOV SI, [200h] and MOV DS, [202h]. The 8086 only supports loading the
  // pointer into the DS or ES segment register.
  case 0x5: {
    const segment_t senum =
        inst.seg_override.value_or(default_segment_for_index(inst.mdrm.mod, inst.mdrm.rm));
    const uint16_t seg = core.sregs.get(senum);
    core.regs.x.set(inst.mdrm.reg, memory.get<uint16_t>(seg, inst.operand16));
    core.sregs.ds = inst.mdrm.reg, memory.get<uint16_t>(seg, inst.operand16 + 2);
  }
    break;
  // RET (far call and clear some bytes of stack)
  case 0xA: {
    core.ip = pop();
    core.sregs.cs = pop();
    core.regs.x.sp += inst.operand16;
  } break;
  // RET (far call)
  case 0xB: {
    core.ip = pop();
    core.sregs.cs = pop();
  } break;
  // "CD":"int imm8",
  case 0xC: call_interrupt(0x03); break;
  case 0xD: call_interrupt(inst.operand8); break;
  }
}

void CPU::execute_0xD(const instruction_t& inst) {
  switch (inst.op & 0x0f) {}
}

void CPU::execute_0xE(const instruction_t& inst) {
  switch (inst.op & 0x0f) {
  // CALL rel16
  case 0x8: {
    // push the next IP onto the stack then jump ahead by the specified offset.
    push(core.ip);
    core.ip += inst.operand16;
  } break;
  }
}

void CPU::execute_0xF(const instruction_t& inst) {
  switch (inst.op & 0x0f) {
  // CLD: Clear directuon flag
  case 0xC: {
    core.flags.clear(DF);
  } break;
  }
}

void CPU::call_interrupt(int num) { 
  if (num == 0x21) {
    fmt::print("DOS Interrupt: 0x{:04x}\r\n", core.regs.x.ax);
    switch (core.regs.h.ah) {
    // display string
    case 0x09: {
      for (auto offset = core.regs.x.dx;; ++offset) {
        const auto m = memory.get<uint8_t>(core.sregs.ds, offset);
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
  } else {
    fmt::print("Interrupt Num: 0x{:02x}\r\n", num);
  }
}

bool CPU::execute(uint16_t start_cs, uint16_t start_ip) {
  core.sregs.cs = start_cs;
  core.ip = start_ip;
  return execute();
}

bool CPU::execute() {
  while (running_) {
    VLOG(2) << "ip: " << core.ip;
    int pos = (core.sregs.cs * 0x10) + core.ip;
    auto inst = decoder.next_instruction(&memory[pos]);
    VLOG(2) << "fetched inst of bytes " << inst.len;
    core.ip += inst.len;
    const auto family = inst.op >> 4;
    switch (family) {
    case 0x0: execute_0x0(inst); break;
    case 0x1: execute_0x1(inst); break;
    case 0x2: execute_0x2(inst); break;
    case 0x3: execute_0x3(inst); break;
    case 0x4: execute_0x4(inst); break;
    case 0x5: execute_0x5(inst); break;
    case 0x6: execute_0x6(inst); break;
    case 0x7: execute_0x7(inst); break;
    case 0x8: execute_0x8(inst); break;
    case 0x9: execute_0x0(inst); break;
    case 0xA: execute_0x0(inst); break;
    case 0xB: execute_0xB(inst); break;
    case 0xC: execute_0xC(inst); break;
    case 0xD: execute_0xC(inst); break;
    case 0xE: execute_0xC(inst); break;
    case 0xF: execute_0xC(inst); break;
    }
    if (VLOG_IS_ON(1)) {
      const auto dispname =
          (inst.metadata.name.empty() ? fmt::format("[{:2X}]", inst.op) : inst.metadata.name);
      if (inst.metadata.name.empty()) {
        LOG(INFO) << "** Unhandled Instruction: " << dispname;
      } else {
        VLOG(1) << "Instruction: " << dispname;
      }
    }
  }
  return true;
}


void CPU::push(uint16_t val) { 
  // To push we decrement the stack pointer and then add it.
  core.regs.x.sp -= 2;
  // TODO(rushfan): assert if sp <= 0x0000
  memory.set<uint16_t>(core.sregs.ss, core.regs.x.sp, val);
}

uint16_t CPU::pop() {
  const auto m = memory.get<uint16_t>(core.sregs.ss, core.regs.x.sp);
  core.regs.x.sp += 2; 
  // TODO(rushfan): assert if sp >= 0xffff
  return m;
}

// https://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetKernighan
static int kern_popcount(int n) {
  int c;
  for (c = 0; n; ++c) {
    n &= (n - 1);
  }
  return c;
}

// this is missing oac
void CPU::parity_szp8(uint8_t oval, uint8_t nval) {
  const auto pc = (kern_popcount(nval) % 2 == 0);
  FLAG_COND(nval & 0x80, core.flags, SF);
  FLAG_COND(nval == 0, core.flags, ZF);
  FLAG_COND(pc, core.flags, PF);
}

void CPU::parity_szp16(uint16_t oval, uint16_t nval) {
  const auto pc = (kern_popcount(nval) % 2 == 0);
  FLAG_COND(nval & 0x8000, core.flags, SF);
  FLAG_COND(nval == 0, core.flags, ZF);
  FLAG_COND(pc, core.flags, PF);
}


} // namespace door86::cpu::x86

