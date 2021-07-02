#include "cpu/x86/cpu.h"
#include "cpu/x86/rmm.h"

#include "core/log.h"
#include "fmt/format.h"
#include <iostream>
#include <iomanip>

namespace door86::cpu::x86 {

CPU::CPU() : core(), decoder(), memory(1 << 20) {}

// TODO(rushfan): Make generic way to set flags after operations
// mostly add
void CPU::execute_0x0(const instruction_t& inst) {
  switch (inst.op & 0x0f) {
  // "0x00":"add r/m8, r8",
  case 0x0: {
    auto rm = rmm8(inst);
    rm += r8(inst);
  } break;
  // "0x01" : "add r/m16/32, r16/32",
  case 0x1: {
    auto rm = rmm16(inst);
    rm += r16(inst);
  } break;
  // 02/r":"add r8, r/m8
  case 0x2: {
    auto r = r8(inst);
    const auto rm = rmm8(inst);
    r += rm;
  } break;
  // 03":"add r16/32, r/m16/32
  case 0x3: {
    auto r = r16(inst);
    auto rm = rmm16(inst);
    r += rm;
  } break;
  // 04":"add al, imm8
  case 0x4: {
    auto r = r8(&core.regs.h.al);
    r += inst.operand8;
  } break;
  // 05":"add ax, imm16/32
  case 0x5: {
    auto r = r16(&core.regs.x.ax);
    r += inst.operand16;
  } break;
  // 06 PUSH ES
  case 0x6: push(core.sregs.es); break;
  // 07 POP ES
  case 0x7: core.sregs.es = pop(); break;
  // 0x08: OR r/m8, r8
  case 0x8: {
    const auto r = r8(inst);
    auto rm = rmm8(inst);
    rm |= r;
  } break;
  // 0x09: OR r/m16, r16
  case 0x9: {
    const auto r = core.regs.x.get(inst.mdrm.reg);
    auto rm = rmm16(inst);
    rm |= r;
  } break;
  // 0x0A: OR r8, r/m8
  case 0xA: {
    auto r = r8(inst);
    const auto rm = rmm8(inst);
    r |= rm;
  } break;
  // 0x0B: OR r16, r/m16
  case 0xB: {
    auto r = r16(inst);
    const auto rm = rmm16(inst);
    r |= rm;
  } break;
  // 0x0C: OR AL, imm8
  case 0xC: {
    auto r = r8(&core.regs.h.al);
    r |= inst.operand8;
  } break;
  // 0x0D: OR AX, imm16
  case 0xD: {
    auto r = r16(&core.regs.x.ax);
    r |= inst.operand16;
  } break;
  // 0x0E: PUSH CS
  case 0xE: push(core.sregs.cs); break;
  // 0x0F: POP CS
  case 0xF: core.sregs.cs = pop(); break;
  }
}

void CPU::execute_0x1(const instruction_t& inst) {
  const bool cf = core.flags.cflag();
  // 0x10-0x15: ADC
  switch (inst.op & 0x0f) {
  // "0x10":"ADC r/m8, r8",
  case 0x0: {
    auto rm = rmm8(inst);
    rm += r8(inst);
    if (cf) {
      rm += 1;
    }
  } break;
  // "0x01" : "add r/m16/32, r16/32",
  case 0x1: {
    auto rm = rmm16(inst);
    rm += r16(inst);
    if (cf) {
      rm += 1;
    }
  } break;
  // 02/r":"add r8, r/m8
  case 0x2: {
    auto r = r8(inst);
    const auto rm = rmm8(inst);
    r += rm;
    if (cf) {
      r += 1;
    }
  } break;
  // 03":"adc r16/32, r/m16/32
  case 0x3: {
    auto r = r16(inst);
    auto rm = rmm16(inst);
    r += rm;
    if (cf) {
      r += 1;
    }
  } break;
  // 14":"ADC al, imm8
  case 0x4: {
    auto r = r8(&core.regs.h.al);
    r += inst.operand8;
    if (cf) {
      r += 1;
    }
  } break;
  // 15":"ADC ax, imm16/32
  case 0x5: {
    auto r = r16(&core.regs.x.ax);
    r += inst.operand16;
    if (cf) {
      r += 1;
    }
  } break;
  // 0x16: PUSH SS
  case 0x6: push(core.sregs.ss); break;
  // 0x17: POP SS
  case 0x7: core.sregs.ss = pop(); break;
  // 0x1E: PUSH DS
  case 0xE: push(core.sregs.ds); break;
  // 0x1F: POP DS
  case 0xF: core.sregs.ds = pop(); break;
  }
}
void CPU::execute_0x2(const instruction_t& inst) {
  switch (inst.op & 0x0f) {
    // 0x22: AND r/m8, r8
  case 0x0: {
    auto rm = rmm8(inst);
    rm &= r8(inst);
  } break;
  // "0x21" : "AND r/m16/32, r16/32",
  case 0x1: {
    auto rm = rmm16(inst);
    rm &= r16(inst);
  } break;
  // 02/r":"AND r8, r/m8
  case 0x2: {
    auto r = r8(inst);
    const auto rm = rmm8(inst);
    r &= rm;
  } break;
  // 23":"AND r16/32, r/m16/32
  case 0x3: {
    auto r = r16(inst);
    auto rm = rmm16(inst);
    r &= rm;
  } break;
  // 24":"AND al, imm8
  case 0x4: {
    auto r = r8(&core.regs.h.al);
    r &= inst.operand8;
  } break;
  // 25":"AND ax, imm16/32
  case 0x5: {
    auto r = r16(&core.regs.x.ax);
    r &= inst.operand16;
  } break;
  }
}

void CPU::execute_0x3(const instruction_t& inst) {
  switch (inst.op & 0x0f) {
  case 0x0: {
    const auto r = core.regs.h.get(inst.mdrm.reg);
    auto rm = rmm8(inst);
    rm ^= r;
  } break;
  case 0x1: {
    const auto r = core.regs.x.get(inst.mdrm.reg);
    auto rm = rmm16(inst);
    rm ^= r;
  } break;
  case 0x2: {
    auto r = r8(inst);
    const auto rm = rmm8(inst);
    r ^= rm;
  } break;
  case 0x3: {
    auto r = r16(inst);
    auto rm = rmm16(inst);
    r ^= rm;
  } break;
  case 0x4: {
    auto r = r8(&core.regs.h.al);
    r ^= inst.operand8;
  } break;
  // 25":"XOR ax, imm16/32
  case 0x5: {
    auto r = r16(&core.regs.x.ax);
    r ^= inst.operand16;
  } break;
  }
}

// INC and DEC
void CPU::execute_0x4(const instruction_t& inst) {
  const auto dec = inst.op & 0x08;
  const auto regnum = inst.op & 0x07;
  auto r = r16(core.regs.x.regptr(regnum));
  if (dec) {
    r -= 1;
  } else {
    r += 1;  
  }
}

// PUSH and POP
void CPU::execute_0x5(const instruction_t& inst) {
  const auto popd = inst.op & 0x08;
  const auto regnum = inst.op & 0x07;
  auto r = r16(core.regs.x.regptr(regnum));
  if (popd) {
    r.set(pop());
  } else {
    push(r.get());
  }
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
    switch (inst.mdrm.reg) {
    case 0x0: {
      auto regmem16 = rmm16(inst);
      regmem16 += inst.operand8;
    } break;
    default: {
      VLOG(1) << "Unhandled submode of 0x83: " << inst.mdrm.reg;
      auto regmem16 = rmm16(inst);
      regmem16 += inst.operand8;
    } break;
    }
  } break;
  // "8A/r":"mov r8, r/m8",
  case 0xA: {
    auto regmem8 = rmm8(inst);
    core.regs.h.set(inst.mdrm.reg, regmem8.get());
  } break;
  // "8D":"lea r16/32, m",
  case 0xD: {
    r16(inst).set(effective_address(inst, core));
  } break;
  // "8E/r":"mov Sreg, r/m16",
  case 0xE: {
    auto regmem16 = rmm16(inst);
    // actually segment per metadata
    r16(inst).set(regmem16.get());
  } break;
  }
}

void CPU::execute_0x9(const instruction_t& inst) {
  switch (inst.op & 0x0f) {}
}

void CPU::execute_0xA(const instruction_t& inst) {
  switch (inst.op & 0x0f) {
  case 0x0: {
    const auto seg = core.sregs.get(inst.seg_index());
    // offset is inst.operand16
    core.regs.h.al = memory.get<uint8_t>(seg, inst.operand8);
  } break;
  case 0x1: {
    const auto seg = core.sregs.get(inst.seg_index());
    // offset is inst.operand16
    core.regs.x.ax = memory.get<uint16_t>(seg, inst.operand16);
  } break;
  case 0x2: {
    const auto seg = core.sregs.get(inst.seg_index());
    // offset is inst.operand16
    memory.set<uint8_t>(seg, inst.operand8, core.regs.h.al);
  } break;
  //  mov [seg:off],ax
  case 0x3: {
    const auto seg = core.sregs.get(inst.seg_index());
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
    const auto seg = core.sregs.get(inst.seg_index());
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
    const auto seg = core.sregs.get(inst.seg_index());
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
    core.flags.dflag(false);
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

// RMM

// returns the offset for the effective address described by a modrm byte
// returns the rm portion for the modrm
Rmm<RmmType::EITHER, uint8_t> CPU::rmm8(const instruction_t& inst) {
  if (inst.mdrm.mod == 0x03) {
    // mod 3 returns a reference to a CPU register
    return Rmm<RmmType::EITHER, uint8_t>(&core, core.regs.h.regptr(inst.mdrm.rm));
  }
  // Pick the overridden segment, or just the default one for the instruction.
  const auto seg = core.sregs.get(inst.seg_index());
  if (inst.mdrm.mod == 0 && inst.mdrm.rm == 0x06) {
    const auto offset =
        inst.metadata.bits == 8 ? inst.operand8 : static_cast<uint8_t>(inst.operand16 & 0xff);
    return Rmm<RmmType::EITHER, uint8_t>(&core, &memory, seg, offset);
  }
  const auto offset = effective_address(inst, core);
  return Rmm<RmmType::EITHER, uint8_t>(&core, &memory, seg, offset);
}

Rmm<RmmType::EITHER, uint16_t> CPU::rmm16(const instruction_t& inst) {
  if (inst.mdrm.mod == 0x03) {
    // mod 3 returns a reference to a CPU register.
    // don't ue r16 since that applies segment override, and ModR/M bytes
    // in mod 3 don't ever use segment registers.
    return Rmm<RmmType::EITHER, uint16_t>(&core, core.regs.x.regptr(inst.mdrm.rm));
  }
  const auto seg = core.sregs.get(inst.seg_index());
  if (inst.mdrm.mod == 0 && inst.mdrm.rm == 0x06) {
    const auto offset = inst.metadata.bits == 8 ? inst.operand8 : inst.operand16;
    return Rmm<RmmType::EITHER, uint16_t>(&core, &memory, seg, offset);
  }
  const auto offset = effective_address(inst, core);
  return Rmm<RmmType::EITHER, uint16_t>(&core, &memory, seg, offset);
}


// returns the register for an r16
Rmm<RmmType::REGISTER, uint8_t> CPU::r8(const instruction_t& inst) {
  return Rmm<RmmType::REGISTER, uint8_t>(&core, core.regs.h.regptr(inst.mdrm.reg));
}

Rmm<RmmType::REGISTER, uint16_t> CPU::r16(const instruction_t& inst) {
  if (inst.metadata.mask & op_mask_reg_is_sreg) {
    return Rmm<RmmType::REGISTER, uint16_t>(&core, core.sregs.regptr(inst.mdrm.reg));
  }
  return Rmm<RmmType::REGISTER, uint16_t>(&core, core.regs.x.regptr(inst.mdrm.reg));
}

Rmm<RmmType::REGISTER, uint8_t> CPU::r8(uint8_t* reg) {
  return Rmm<RmmType::REGISTER, uint8_t>(&core, reg);
}

Rmm<RmmType::REGISTER, uint16_t> CPU::r16(uint16_t* reg) {
  return Rmm<RmmType::REGISTER, uint16_t>(&core, reg);
}


} // namespace door86::cpu::x86
