#include "cpu/x86/cpu.h"
#include "cpu/x86/rmm.h"

#include "core/log.h"
#include "fmt/format.h"
#include <iostream>
#include <iomanip>

#ifdef _MSC_VER
#include <intrin.h>
#elif __GNUC__
#endif 

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
  
  // 0x18: SBB r/m8	r8
  case 0x8: {
    auto rm = rmm8(inst);
    rm -= r8(inst);
    if (cf) {
      rm -= 1;
    }
  } break;
  // "0x19" : "SBB r/m16/32, r16/32",
  case 0x9: {
    auto rm = rmm16(inst);
    rm -= r16(inst);
    if (cf) {
      rm -= 1;
    }
  } break;
  // 0x1A":"SBB r8, r/m8
  case 0xa: {
    auto r = r8(inst);
    const auto rm = rmm8(inst);
    r -= rm;
    if (cf) {
      r -= 1;
    }
  } break;
  // 1B":"SBB r16/32, r/m16/32
  case 0xb: {
    auto r = r16(inst);
    auto rm = rmm16(inst);
    r -= rm;
    if (cf) {
      r -= 1;
    }
  } break;
  // 1C":"SBB al, imm8
  case 0xc: {
    auto r = r8(&core.regs.h.al);
    r -= inst.operand8;
    if (cf) {
      r -= 1;
    }
  } break;
  // 1D":"SBB ax, imm16/32
  case 0xd: {
    auto r = r16(&core.regs.x.ax);
    r -= inst.operand16;
    if (cf) {
      r -= 1;
    }
  } break;
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

  // 0x28: SUB r/m8, r8
  case 0x8: {
    const auto r = r8(inst);
    auto rm = rmm8(inst);
    rm -= r;
  } break;
  // 0xs9: SUB r/m16, r16
  case 0x9: {
    const auto r = core.regs.x.get(inst.mdrm.reg);
    auto rm = rmm16(inst);
    rm -= r;
  } break;
  // 0xsA: SUB r8, r/m8
  case 0xA: {
    auto r = r8(inst);
    const auto rm = rmm8(inst);
    r -= rm;
  } break;
  // 0xsB: SUB r16, r/m16
  case 0xB: {
    auto r = r16(inst);
    const auto rm = rmm16(inst);
    r -= rm;
  } break;
  // 0xsC: SUB AL, imm8
  case 0xC: {
    auto r = r8(&core.regs.h.al);
    r -= inst.operand8;
  } break;
  // 0xsD: SUB AX, imm16
  case 0xD: {
    auto r = r16(&core.regs.x.ax);
    r -= inst.operand16;
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
  // 38 /r CMP r/m8, r8
  case 0x8: {
    auto rm = rmm8(inst);
    const auto r = r8(inst);
    rm.cmp(r);
  } break;
  // 39 /r CMP r/m16, r16
  case 0x9: {
    auto rm = rmm16(inst);
    const auto r = r16(inst);
    rm.cmp(r);
  } break;
  // 3A /r CMP r8, r/m8
  case 0xA: {
    auto r = r8(inst);
    const auto rm = rmm8(inst);
    r.cmp(rm);
  } break;
  // 3B /r CMP r16, r/m16
  case 0xB: {
    auto r = r16(inst);
    const auto rm = rmm16(inst);
    r.cmp(rm);
  } break;
  // 3C ib CMP AL, imm8
  case 0xC: {
    auto r = r8(&core.regs.h.al);
    r.cmp(inst.operand8);
  } break;
  // 3D iw CMP AX, imm16
  case 0xD: {
    auto r = r16(&core.regs.x.ax);
    r.cmp(inst.operand16);
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
  switch (inst.op & 0x0f) {
  // PUSHA
  case 0x0: {
    push(core.regs.x.ax);
    push(core.regs.x.cx);
    push(core.regs.x.dx);
    push(core.regs.x.bx);
    push(core.regs.x.sp);
    push(core.regs.x.bp);
    push(core.regs.x.si);
    push(core.regs.x.di);
  } break;
  // POPA
  case 0x1: {
    core.regs.x.di = pop();
    core.regs.x.si = pop();
    core.regs.x.bp = pop();
    core.regs.x.sp = pop();
    core.regs.x.bx = pop();
    core.regs.x.dx = pop();
    core.regs.x.cx = pop();
    core.regs.x.ax = pop();
  } break;
  }
}

void CPU::execute_0x7(const instruction_t& inst) {
  bool cond = false;
  switch (inst.op & 0x0f) {
  case 0x0: cond = core.flags.oflag(); break;
  case 0x1: cond = !core.flags.oflag(); break;
  case 0x2: cond = core.flags.cflag(); break;
  case 0x3: cond = !core.flags.oflag(); break;
  case 0x4: cond = core.flags.zflag(); break;
  case 0x5: cond = !core.flags.zflag(); break;
  case 0x6: cond = core.flags.cflag() && core.flags.zflag(); break;
  case 0x7: cond = !core.flags.cflag() && !core.flags.zflag(); break;
  case 0x8: cond = core.flags.sflag(); break;
  case 0x9: cond = !core.flags.sflag(); break;
  case 0xa: cond = core.flags.pflag(); break;
  case 0xb: cond = !core.flags.pflag(); break;
  case 0xc: cond = core.flags.sflag() != core.flags.oflag(); break;
  case 0xd: cond = core.flags.sflag() == core.flags.oflag(); break;
  case 0xe: cond = core.flags.zflag() && (core.flags.sflag() != core.flags.oflag()); break;
  case 0xf: cond = !core.flags.zflag() && (core.flags.sflag() == core.flags.oflag()); break;
  }
  if (cond) {
    core.ip += inst.operand8;
  }
}

void CPU::execute_0x8(const instruction_t& inst) {
  switch (inst.op & 0x0f) {
  // 0x80/M
  case 0x0: {
    switch (inst.mdrm.reg) {
    // 80 /0 ib: ADD r/m8, imm8
    case 0x0: {
      auto rm = rmm8(inst);
      rm += inst.operand8;
    } break;
    // 80 /1 ib: OR r/m8, imm8
    case 0x1: {
      auto rm = rmm8(inst);
      rm |= inst.operand8;
    } break;
    // 80 /2 ib: ADS r/m8, imm8
    case 0x2: {
      const auto cf = core.flags.cflag();
      auto rm = rmm8(inst);
      rm += inst.operand8;
      if (cf) {
        rm += 1;
      }
    } break;
    // 80 /3 ib: SBB r/m8, imm8
    case 0x3: {
      const auto cf = core.flags.cflag();
      auto rm = rmm8(inst);
      rm -= inst.operand8;
      if (cf) {
        rm -= 1;
      }
    } break;
    // 80 /4 ib: AND r/m8, imm8
    case 0x4: {
      const auto cf = core.flags.cflag();
      auto rm = rmm8(inst);
      rm &= inst.operand8;
    } break;
    // 80 /5 ib: SUB r/m8, imm8
    case 0x5: {
      const auto cf = core.flags.cflag();
      auto rm = rmm8(inst);
      rm -= inst.operand8;
    } break;
    // 80 /6 ib: XOR r/m8, imm8
    case 0x6: {
      const auto cf = core.flags.cflag();
      auto rm = rmm8(inst);
      rm ^= inst.operand8;
    } break;
    // 80 /7 ib: CMP r/m8, imm8
    case 0x7: {
      auto rm = rmm8(inst);
      rm.cmp(inst.operand8);
    } break;
    default: {
      VLOG(1) << "Unhandled submode of 0x83: " << inst.mdrm.reg;
    } break;
    }
  } break;
  // 0x81/M
  case 0x1: {
    switch (inst.mdrm.reg) {
    // 81 /0 iw: ADD r/m16, imm16
    case 0x0: {
      auto rm = rmm16(inst);
      rm += inst.operand16;
    } break;
    // 81 /1 i2: OR r/m16, imm16
    case 0x1: {
      auto rm = rmm16(inst);
      rm |= inst.operand16;
    } break;
    // 81 /2 iw: ADS r/m16, imm16
    case 0x2: {
      const auto cf = core.flags.cflag();
      auto rm = rmm16(inst);
      rm += inst.operand16;
      if (cf) {
        rm += 1;
      }
    } break;
    // 81 /3 iw: SBB r/m16, imm16
    case 0x3: {
      const auto cf = core.flags.cflag();
      auto rm = rmm16(inst);
      rm -= inst.operand16;
      if (cf) {
        rm -= 1;
      }
    } break;
    // 81 /4 iw: AND r/m16, imm16
    case 0x4: {
      const auto cf = core.flags.cflag();
      auto rm = rmm16(inst);
      rm &= inst.operand16;
    } break;
    // 81 /5 iw: SUB r/m16, imm16
    case 0x5: {
      const auto cf = core.flags.cflag();
      auto rm = rmm16(inst);
      rm -= inst.operand16;
    } break;
    // 81 /6 iw: XOR r/m16, imm16
    case 0x6: {
      const auto cf = core.flags.cflag();
      auto rm = rmm16(inst);
      rm ^= inst.operand16;
    } break;
    // 81 /7 iw: CMP r/m16, imm16
    case 0x7: {
      auto rm = rmm16(inst);
      rm.cmp(inst.operand16);
    } break;
    default: {
      VLOG(1) << "Unhandled submode of 0x83: " << inst.mdrm.reg;
    } break;
    }
  } break; 
  // 0x83/M
  case 0x3: {
    // only add if reg == 0
    switch (inst.mdrm.reg) {
    // 83/0":"add r/m16/32, imm8
    case 0x0: {
      auto regmem16 = rmm16(inst);
      regmem16 += static_cast<uint16_t>(inst.operand8);
    } break;
    // 83 /1 i2: OR r/m16, imm8
    case 0x1: {
      auto rm = rmm16(inst);
      rm |= static_cast<uint16_t>(inst.operand8);
    } break;
    // 83 /2 iw: ADS r/m16, imm8
    case 0x2: {
      const auto cf = core.flags.cflag();
      auto rm = rmm16(inst);
      rm += static_cast<uint16_t>(inst.operand8);
      if (cf) {
        rm += 1;
      }
    } break;
    // 83 /3 iw: SBB r/m16, imm8
    case 0x3: {
      const auto cf = core.flags.cflag();
      auto rm = rmm16(inst);
      rm -= static_cast<uint16_t>(inst.operand8);
      if (cf) {
        rm -= 1;
      }
    } break;
    // 83 /4 iw: AND r/m16, imm8
    case 0x4: {
      const auto cf = core.flags.cflag();
      auto rm = rmm16(inst);
      rm &= static_cast<uint16_t>(inst.operand8);
    } break;
    // 83 /5 iw: SUB r/m16, imm8
    case 0x5: {
      const auto cf = core.flags.cflag();
      auto rm = rmm16(inst);
      rm -= static_cast<uint16_t>(inst.operand8);
    } break;
    // 83 /6 iw: XOR r/m16, imm8
    case 0x6: {
      const auto cf = core.flags.cflag();
      auto rm = rmm16(inst);
      rm ^= static_cast<uint16_t>(inst.operand8);
    } break;
    // 83 /7 iw: CMP r/m16, imm8
    case 0x7: {
      auto rm = rmm16(inst);
      rm.cmp(static_cast<uint16_t>(inst.operand8));
    } break;
    default: {
      VLOG(1) << "Unhandled submode of 0x83: " << inst.mdrm.reg;
    } break;
    }
  } break;
  // 0x84 /r: TEST r/m8, r8
  case 0x4: {
    auto rmm = rmm8(inst);
    const auto saved_rmm = rmm.get();
    auto r = r8(inst);
    rmm &= r;
    rmm.set(r.get());
  } break;
  // 0x85 /r: TEST r/m16, r16
  case 0x5: {
    auto rmm = rmm16(inst);
    const auto saved_rmm = rmm.get();
    auto r = r16(inst);
    rmm &= r;
    rmm.set(r.get());
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

void CPU::scas_m8(const instruction_t& inst) {
  const int16_t step = core.flags.dflag() ? -1 : 1;
  const auto b = memory.get<uint8_t>(core.sregs.es, core.regs.x.di);
  auto r = r8(&core.regs.h.ah);
  r.cmp(b);
  core.regs.x.di += step;
}

void CPU::scas_m16(const instruction_t& inst) {
  const int16_t step = core.flags.dflag() ? -1 : 1;
  const auto b = memory.get<uint16_t>(core.sregs.es, core.regs.x.di);
  auto r = r16(&core.regs.x.ax);
  r.cmp(b);
  core.regs.x.di += step;
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
  // A8 ib: TEST AL, imm8
  case 0x8: {
    uint8_t operand{inst.operand8};
    Rmm<RmmType::REGISTER, uint8_t> op(&core, &operand);
    auto r = r8(&core.regs.h.al);
    op &= r;
  } break;
  // A9 iw: TEST AX, imm16
  case 0x9: {
    uint16_t operand{inst.operand16};
    Rmm<RmmType::REGISTER, uint16_t> op(&core, &operand);
    auto r = r16(&core.regs.x.ax);
    op &= r;
  } break;
  // SCAS m8
  case 0xE: {
    scas_m8(inst);
  } break;
  // SCAS m16
  case 0xF: {
    scas_m16(inst);
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
  case 0x0: return execute_0xC0(inst, inst.mdrm.reg);
  case 0x1: return execute_0xC1(inst, inst.mdrm.reg);
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
  } break;
  // MOV r/m8, imm8
  case 0x6: {
    auto rm = rmm8(inst);
    rm.set(inst.operand8);
  } break;
  // MOV r/m16, imm16
  case 0x7: {
    auto rm = rmm16(inst);
    rm.set(inst.operand16);
  } break;
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

void CPU::execute_0xC0(const instruction_t& inst, int subop) {
  switch (subop) {}
}

void CPU::execute_0xC1(const instruction_t& inst, int subop) {
  switch (subop) {}
}

void CPU::execute_0xD(const instruction_t& inst) {
  switch (inst.op & 0x0f) {}
}

void CPU::execute_0xE(const instruction_t& inst) {
  switch (inst.op & 0x0f) {
  case 0x3: {
    if (core.regs.x.cx == 0) {
      core.ip += inst.operand8;
    }
  } break;
  // CALL rel16
  case 0x8: {
    // push the next IP onto the stack then jump ahead by the specified offset.
    push(core.ip);
    core.ip += inst.operand16;
  } break;
  // JMP rel16
  case 0x9: {
    core.ip += inst.operand16;
  } break;
  // JMP rel8
  case 0xB: {
    core.ip += inst.operand8;
  } break;
  }
}

void CPU::execute_0xF(const instruction_t& inst) {
  switch (inst.op & 0x0f) {
  // 0xF6/M
  case 0x6: {
    switch (inst.mdrm.reg) {
    case 0: {
      uint8_t operand{inst.operand8};
      Rmm<RmmType::REGISTER, uint8_t> op(&core, &operand);
      auto rm = rmm8(inst);
      op &= rm;
    } break;
    // NOT r/m8
    case 2: {
      auto rm = rmm8(inst);
      const auto n = rm.get();
      rm.set(~n);
    } break;
    // NEG r/m8
    case 3: {
      auto rm = rmm8(inst);
      rm.neg();
    } break;
    }
  } break;
  // F6 /4 MUL r/m8
  case 0x4: {
    core.regs.x.ax = core.regs.h.al * inst.operand8;
    core.flags.oflag(core.regs.h.ah);
    core.flags.cflag(core.regs.h.ah);
  } break;
  // F6 /5 IMUL r/m8
  case 0x5: {
    core.regs.x.ax = core.regs.h.al * inst.operand8;
    core.flags.oflag((core.regs.h.al & 0x80) && core.regs.h.ah);
    core.flags.cflag((core.regs.h.al & 0x80) && core.regs.h.ah);
  } break;
  // 0xF7/M
  case 0x7: {
    switch (inst.mdrm.reg) {
    case 0: {
      uint16_t operand{inst.operand16};
      Rmm<RmmType::REGISTER, uint16_t> op(&core, &operand);
      auto rm = rmm16(inst);
      op &= rm;
    } break;
    // NOT r/m16
    case 2: {
      auto rm = rmm16(inst);
      const auto n = rm.get();
      rm.set(~n);
    } break;
    // NEG r/m16
    case 3: {
      auto rm = rmm16(inst);
      rm.neg();
    } break;
    // F7 /4 MUL r/m16
    case 0x4: {
      uint32_t result = core.regs.x.ax * inst.operand16;
      core.regs.x.dx = ((result & 0xFF00) >> 16);
      core.flags.oflag((core.regs.x.ax & 0x8000) && core.regs.x.dx);
      core.flags.cflag((core.regs.x.ax & 0x8000) && core.regs.x.dx);
    } break;
    // F7 /5 IMUL r/m16
    case 0x5: {
      uint32_t result = core.regs.x.ax * inst.operand16;
      core.regs.x.dx = ((result & 0xFF00) >> 16);
      core.flags.oflag(core.regs.x.dx);
      core.flags.cflag(core.regs.x.dx);
    } break;
    }
  } break;
  // CLD: Clear directuon flag
  case 0xC: {
    core.flags.dflag(false);
  } break;
  // STD—Set Direction Flag
  case 0xD: {
    core.flags.dflag(true);
  } break;
  // INC or DEC r/m8
  case 0xE: {
    switch (inst.mdrm.reg) {
    case 0: { // INC
      auto r = rmm8(inst);
      r += 1;
    } break;
    case 1: { // DEC
      auto r = rmm8(inst);
      r -= 1;
    } break;
    default: {
      LOG(WARNING) << "Unhandled subcode of opcode: 0xFE; subcode: " << static_cast<int>(inst.mdrm.reg);
    } break;
    }
  } break;
  // INC r/m16
  case 0xF: {
    switch (inst.mdrm.reg) {
    case 0: { // INC
      auto r = rmm16(inst);
      r += 1;
    } break;
    case 1: { // DEC
      auto r = rmm16(inst);
      r -= 1;
    } break;
    case 2: { // CALL
      // push the next IP onto the stack then jump ahead by the specified offset.
      push(core.ip);
      core.ip = inst.operand16;
    } break;
    // JMP r/m16
    case 4: {
      core.ip = inst.operand16;
    } break;
    // 5: JMP m16:16
    // TODO(rushfan): Need an example of this.
    //case 5: {
    //  const auto current_seg = core.sregs.get(inst.seg_index());
    //  const auto offset = memory.get<uint16_t>(seg, inst.operand16);
    //  const auto seg = memory.get<uint16_t>(seg, inst.operand16 + 2);
    //  core.ip = inst.operand16;
    //} break;
    default: {
      LOG(WARNING) << "Unhandled subcode of opcode: 0xFF; subcode: "
                   << static_cast<int>(inst.mdrm.reg);
    } break;
    }
  } break;
  }
}

void CPU::call_interrupt(int num) {
  if (auto fn = int_handlers_.find(num); fn != std::end(int_handlers_)) {
    fn->second(num, *this);
    return;
  }
  // static default handlers
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
    default: {
      // unhandled
    } break;
    }
  } else if (num >= 0x86 && num <= 0xF0) {
    // INT 86 to F0: INT 86 to F0 - used by BASIC while in interpreter
    running_ = false;
    LOG(INFO) << "Exiting; Out of band Interrupt Num: 0x" << std::hex << num;
  } else {
    LOG(INFO) << "Unhandled Interrupt Num: 0x" << std::hex << num;
  }
}

bool CPU::run(uint16_t start_cs, uint16_t start_ip) {
  core.sregs.cs = start_cs;
  core.ip = start_ip;
  return run();
}

bool CPU::run() {
  while (running_) {
    VLOG(2) << "ip: " << core.ip;
    const int pos = (core.sregs.cs * 0x10) + core.ip;
    const auto inst = decoder.decode(&memory[pos]);
    VLOG(2) << "fetched inst of bytes " << inst.len;
    core.ip += inst.len;
    execute(inst);
  }
  return true;
}

bool CPU::execute(const instruction_t& inst) {
  if (inst.metadata.mask & op_mask_notimpl) {
    LOG(WARNING) << "Unimplemented Opcode Encountered: " << std::hex << static_cast<uint16_t>(inst.op);
    return false;
  }
  bool done = true;
  do {
    switch (inst.op >> 4) {
    case 0x0: execute_0x0(inst); break;
    case 0x1: execute_0x1(inst); break;
    case 0x2: execute_0x2(inst); break;
    case 0x3: execute_0x3(inst); break;
    case 0x4: execute_0x4(inst); break;
    case 0x5: execute_0x5(inst); break;
    case 0x6: execute_0x6(inst); break;
    case 0x7: execute_0x7(inst); break;
    case 0x8: execute_0x8(inst); break;
    case 0x9: execute_0x9(inst); break;
    case 0xA: execute_0xA(inst); break;
    case 0xB: execute_0xB(inst); break;
    case 0xC: execute_0xC(inst); break;
    case 0xD: execute_0xD(inst); break;
    case 0xE: execute_0xE(inst); break;
    case 0xF: execute_0xF(inst); break;
    }
    if (inst.rep) {
      done = false;
      if (--core.regs.x.cx == 0) {
        done = true;
        break;
      }
      if (inst.metadata.uses_rep_zf) {
        done = core.flags.zflag();    
      }
    } else if (inst.repne) {
      if (--core.regs.x.cx == 0) {
        done = true;
        break;
      }
      if (inst.metadata.uses_rep_zf) {
        done = !core.flags.zflag();
      }
    }
  } while (!done);
  if (VLOG_IS_ON(1)) {
    const auto dispname =
        (inst.metadata.name.empty() ? fmt::format("[{:2X}]", inst.op) : inst.metadata.name);
    if (inst.metadata.name.empty()) {
      LOG(INFO) << "** Unhandled Instruction: " << dispname;
    } else {
      VLOG(1) << "Instruction: " << dispname;
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


// REPEAT SUPPORT

void CPU::rep(const instruction_t& inst) {
  if (!inst.rep && !inst.repne) {
    LOG(WARNING) << "Called CPU::rep without a REP or REPNE prefix";
    return;
  }
  if (inst.rep && inst.repne) {
    LOG(WARNING) << "Called CPU::rep with *BOTH* a REP or REPNE prefix";
    return;
  }

  switch (inst.op) {
  // SCASB
  case 0xAE: {
    for (; core.flags.zflag() == inst.repne && core.regs.x.cx; --core.regs.x.cx) {
      scas_m8(inst);
    }
  } break;
  case 0xAF: {
    for (; core.flags.zflag() == inst.repne && core.regs.x.cx; --core.regs.x.cx) {
      scas_m16(inst);
    }
  } break;
  default: {
    LOG(WARNING) << "Unhandled REP opcode: " << std::hex << inst.op;
  } break;
  }
}


} // namespace door86::cpu::x86

