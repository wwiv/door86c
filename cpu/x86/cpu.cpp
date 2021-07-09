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
    r += inst.imm8;
  } break;
  // 05":"add ax, imm16/32
  case 0x5: {
    auto r = r16(&core.regs.x.ax);
    r += inst.imm16;
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
    r |= inst.imm8;
  } break;
  // 0x0D: OR AX, imm16
  case 0xD: {
    auto r = r16(&core.regs.x.ax);
    r |= inst.imm16;
  } break;
  // 0x0E: PUSH CS
  case 0xE: push(core.sregs.cs); break;
  // 0x0F: POP CS
  case 0xF: core.sregs.cs = pop(); break;
  default:
    LOG(WARNING) << fmt::format("Skipped OPCODE: 0x{:02x}", static_cast<int>(inst.op));
    break;
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
    r += inst.imm8;
    if (cf) {
      r += 1;
    }
  } break;
  // 15":"ADC ax, imm16/32
  case 0x5: {
    auto r = r16(&core.regs.x.ax);
    r += inst.imm16;
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
    r -= inst.imm8;
    if (cf) {
      r -= 1;
    }
  } break;
  // 1D":"SBB ax, imm16/32
  case 0xd: {
    auto r = r16(&core.regs.x.ax);
    r -= inst.imm16;
    if (cf) {
      r -= 1;
    }
  } break;
  default: LOG(WARNING) << fmt::format("Skipped OPCODE: 0x{:02x}", static_cast<int>(inst.op)); break;
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
    r &= inst.imm8;
  } break;
  // 25":"AND ax, imm16/32
  case 0x5: {
    auto r = r16(&core.regs.x.ax);
    r &= inst.imm16;
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
  // 0x2A: SUB r8, r/m8
  case 0xA: {
    auto r = r8(inst);
    const auto rm = rmm8(inst);
    r -= rm;
  } break;
  // 0x2B: SUB r16, r/m16
  case 0xB: {
    auto r = r16(inst);
    const auto rm = rmm16(inst);
    r -= rm;
  } break;
  // 0x2C: SUB AL, imm8
  case 0xC: {
    auto r = r8(&core.regs.h.al);
    r -= inst.imm8;
  } break;
  // 0x2D: SUB AX, imm16
  case 0xD: {
    auto r = r16(&core.regs.x.ax);
    r -= inst.imm16;
  } break;
  default:
    LOG(WARNING) << fmt::format("Skipped OPCODE: 0x{:02x}", static_cast<int>(inst.op));
    break;
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
    r ^= inst.imm8;
  } break;
  // 25":"XOR ax, imm16/32
  case 0x5: {
    auto r = r16(&core.regs.x.ax);
    r ^= inst.imm16;
  } break;
  // 38 /r CMP r/m8, r8
  case 0x8: {
    const auto rm = rmm8(inst);
    auto r = r8(inst);
    r.cmp(rm.get());
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
    r.cmp(inst.imm8);
  } break;
  // 3D iw CMP AX, imm16
  case 0xD: {
    auto r = r16(&core.regs.x.ax);
    r.cmp(inst.imm16);
  } break;
  default:
    LOG(WARNING) << fmt::format("Skipped OPCODE: 0x{:02x}", static_cast<int>(inst.op));
    break;
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
    const auto temp = core.regs.x.sp;
    push(core.regs.x.ax);
    push(core.regs.x.cx);
    push(core.regs.x.dx);
    push(core.regs.x.bx);
    push(temp);
    push(core.regs.x.bp);
    push(core.regs.x.si);
    push(core.regs.x.di);
    VLOG(1) << fmt::format("PUSHA; SP: {:02x}", core.regs.x.sp);
  } break;
  // POPA
  case 0x1: {
    core.regs.x.di = pop();
    core.regs.x.si = pop();
    core.regs.x.bp = pop();
    // skip core.regs.x.sp
    core.regs.x.sp += 2;
    core.regs.x.bx = pop();
    core.regs.x.dx = pop();
    core.regs.x.cx = pop();
    core.regs.x.ax = pop();
    VLOG(1) << fmt::format("POPA; SP: {:02x}", core.regs.x.sp);
  } break;
  case 0x8: push(inst.imm16); break;
  case 0xA: push(inst.imm8); break;
  default:
    LOG(WARNING) << fmt::format("Skipped OPCODE: 0x{:02x}", static_cast<int>(inst.op));
    break;
  }
}

void CPU::execute_0x7(const instruction_t& inst) {
  bool cond = false;
  switch (inst.op & 0x0f) {
  case 0x0: cond = core.flags.oflag(); break;
  case 0x1: cond = !core.flags.oflag(); break;
  case 0x2: cond = core.flags.cflag(); break;
  case 0x3: cond = !core.flags.cflag(); break;
  case 0x4: cond = core.flags.zflag(); break;
  case 0x5: cond = !core.flags.zflag(); break;
  case 0x6: cond = core.flags.cflag() || core.flags.zflag(); break;
  case 0x7: cond = !core.flags.cflag() && !core.flags.zflag(); break;
  case 0x8: cond = core.flags.sflag(); break;
  case 0x9: cond = !core.flags.sflag(); break;
  case 0xa: cond = core.flags.pflag(); break;
  case 0xb: cond = !core.flags.pflag(); break;
  case 0xc: cond = core.flags.sflag() != core.flags.oflag(); break;
  case 0xd: cond = core.flags.sflag() == core.flags.oflag(); break;
  case 0xe: cond = core.flags.zflag() || (core.flags.sflag() != core.flags.oflag()); break;
  case 0xf: cond = !core.flags.zflag() && (core.flags.sflag() == core.flags.oflag()); break;
  default:
    LOG(WARNING) << fmt::format("Skipped OPCODE: 0x{:02x}", static_cast<int>(inst.op));
    break;
  }
  if (cond) {
    const int8_t rel8 = static_cast<int8_t>(inst.imm8);
    core.ip += rel8;
  }
}

void CPU::execute_0x80(const instruction_t& inst, int subop) {
  switch (inst.mdrm.reg) {
  // 80 /0 ib: ADD r/m8, imm8
  case 0x0: {
    auto rm = rmm8(inst);
    rm += inst.imm8;
  } break;
  // 80 /1 ib: OR r/m8, imm8
  case 0x1: {
    auto rm = rmm8(inst);
    rm |= inst.imm8;
  } break;
  // 80 /2 ib: ADS r/m8, imm8
  case 0x2: {
    const auto cf = core.flags.cflag();
    auto rm = rmm8(inst);
    rm += inst.imm8;
    if (cf) {
      rm += 1;
    }
  } break;
  // 80 /3 ib: SBB r/m8, imm8
  case 0x3: {
    const auto cf = core.flags.cflag();
    auto rm = rmm8(inst);
    rm -= inst.imm8;
    if (cf) {
      rm -= 1;
    }
  } break;
  // 80 /4 ib: AND r/m8, imm8
  case 0x4: {
    const auto cf = core.flags.cflag();
    auto rm = rmm8(inst);
    rm &= inst.imm8;
  } break;
  // 80 /5 ib: SUB r/m8, imm8
  case 0x5: {
    const auto cf = core.flags.cflag();
    auto rm = rmm8(inst);
    rm -= inst.imm8;
  } break;
  // 80 /6 ib: XOR r/m8, imm8
  case 0x6: {
    const auto cf = core.flags.cflag();
    auto rm = rmm8(inst);
    rm ^= inst.imm8;
  } break;
  // 80 /7 ib: CMP r/m8, imm8
  case 0x7: {
    auto rm = rmm8(inst);
    rm.cmp(inst.imm8);
  } break;
  default: {
    VLOG(1) << "Unhandled submode of 0x83: " << inst.mdrm.reg;
  } break;
  }
}

void CPU::execute_0x81(const instruction_t& inst, int subop) {
  switch (inst.mdrm.reg) {
  // 81 /0 iw: ADD r/m16, imm16
  case 0x0: {
    auto rm = rmm16(inst);
    rm += inst.imm16;
  } break;
  // 81 /1 i2: OR r/m16, imm16
  case 0x1: {
    auto rm = rmm16(inst);
    rm |= inst.imm16;
  } break;
  // 81 /2 iw: ADS r/m16, imm16
  case 0x2: {
    const auto cf = core.flags.cflag();
    auto rm = rmm16(inst);
    rm += inst.imm16;
    if (cf) {
      rm += 1;
    }
  } break;
  // 81 /3 iw: SBB r/m16, imm16
  case 0x3: {
    const auto cf = core.flags.cflag();
    auto rm = rmm16(inst);
    rm -= inst.imm16;
    if (cf) {
      rm -= 1;
    }
  } break;
  // 81 /4 iw: AND r/m16, imm16
  case 0x4: {
    const auto cf = core.flags.cflag();
    auto rm = rmm16(inst);
    rm &= inst.imm16;
  } break;
  // 81 /5 iw: SUB r/m16, imm16
  case 0x5: {
    const auto cf = core.flags.cflag();
    auto rm = rmm16(inst);
    rm -= inst.imm16;
  } break;
  // 81 /6 iw: XOR r/m16, imm16
  case 0x6: {
    const auto cf = core.flags.cflag();
    auto rm = rmm16(inst);
    rm ^= inst.imm16;
  } break;
  // 81 /7 iw: CMP r/m16, imm16
  case 0x7: {
    auto rm = rmm16(inst);
    rm.cmp(inst.imm16);
  } break;
  default: {
    VLOG(1) << "Unhandled submode of 0x83: " << inst.mdrm.reg;
  } break;
  }
}

void CPU::execute_0x82(const instruction_t& inst, int subop) { LOG(FATAL) << "Implement me"; }

void CPU::execute_0x83(const instruction_t& inst, int subop) {
  // only add if reg == 0
  switch (inst.mdrm.reg) {
  // 83/0":"add r/m16/32, imm8
  case 0x0: {
    auto regmem16 = rmm16(inst);
    regmem16 += static_cast<uint16_t>(inst.imm8);
  } break;
  // 83 /1 i2: OR r/m16, imm8
  case 0x1: {
    auto rm = rmm16(inst);
    rm |= static_cast<uint16_t>(inst.imm8);
  } break;
  // 83 /2 iw: ADS r/m16, imm8
  case 0x2: {
    const auto cf = core.flags.cflag();
    auto rm = rmm16(inst);
    rm += static_cast<uint16_t>(inst.imm8);
    if (cf) {
      rm += 1;
    }
  } break;
  // 83 /3 iw: SBB r/m16, imm8
  case 0x3: {
    const auto cf = core.flags.cflag();
    auto rm = rmm16(inst);
    rm -= static_cast<uint16_t>(inst.imm8);
    if (cf) {
      rm -= 1;
    }
  } break;
  // 83 /4 iw: AND r/m16, imm8
  case 0x4: {
    const auto cf = core.flags.cflag();
    auto rm = rmm16(inst);
    rm &= static_cast<uint16_t>(inst.imm8);
  } break;
  // 83 /5 iw: SUB r/m16, imm8
  case 0x5: {
    const auto cf = core.flags.cflag();
    auto rm = rmm16(inst);
    rm -= static_cast<uint16_t>(inst.imm8);
  } break;
  // 83 /6 iw: XOR r/m16, imm8
  case 0x6: {
    const auto cf = core.flags.cflag();
    auto rm = rmm16(inst);
    rm ^= static_cast<uint16_t>(inst.imm8);
  } break;
  // 83 /7 iw: CMP r/m16, imm8
  case 0x7: {
    auto rm = rmm16(inst);
    rm.cmp(static_cast<uint16_t>(inst.imm8));
  } break;
  default: {
    VLOG(1) << "Unhandled submode of 0x83: " << inst.mdrm.reg;
  } break;
  }
}

void CPU::execute_0x8(const instruction_t& inst) {
  switch (inst.op & 0x0f) {
  // 0x80/M
  case 0x0: execute_0x80(inst, inst.mdrm.reg); break;
  // 0x81/M
  case 0x1: execute_0x81(inst, inst.mdrm.reg); break; 
  // 0x83/M
  case 0x3: execute_0x83(inst, inst.mdrm.reg); break;
  // 0x84 /r: TEST r/m8, r8
  case 0x4: {
    auto rmm = rmm8(inst);
    const auto saved_rmm = rmm.get();
    auto r = r8(inst);
    rmm &= r;
    rmm.set(saved_rmm);
  } break;
  // 0x85 /r: TEST r/m16, r16
  case 0x5: {
    auto rmm = rmm16(inst);
    const auto saved_rmm = rmm.get();
    auto r = r16(inst);
    rmm &= r;
    rmm.set(saved_rmm);
  } break;
  // XCHG r8, r/m8
  case 0x6: {
    auto r = r8(inst);
    auto rmm = rmm8(inst);
    swap(r, rmm);
  } break;
  // XCHG r16, r/m16
  case 0x7: {
    auto r = r16(inst);
    auto rmm = rmm16(inst);
    swap(r, rmm);
  } break;
  // MOV r/m8,r8
  case 0x8: {
    auto r = r8(inst);
    auto rmm = rmm8(inst);
    rmm.set(r.get());
  } break;
  // MOV r/m16,r16
  case 0x9: {
    auto r = r16(inst);
    auto rmm = rmm16(inst);
    rmm.set(r.get());
  } break;
  // "8A/r":"mov r8, r/m8",
  case 0xA: {
    auto regmem8 = rmm8(inst);
    core.regs.h.set(inst.mdrm.reg, regmem8.get());
  } break;
  // "8B/r":MOV r16,r/m16
  case 0xB: {
    auto rmm = rmm16(inst);
    auto r = r16(inst);
    r.set(rmm.get());
  } break;
  // 8C/r: MOV r/m16,Sreg**
  case 0xC: {
    auto rmm = rmm16(inst);
    auto sreg = core.sregs.get(inst.mdrm.reg);
    rmm.set(sreg);
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
  // POP r/m16
  case 0xF: {
    if (inst.mdrm.reg != 0) {
      LOG(WARNING) << "Got 8F/N where N>0";
    }
    auto rm = rmm16(inst);
    rm.set(pop());
  } break;
  default:
    LOG(WARNING) << fmt::format("Skipped OPCODE: 0x{:02x}", static_cast<int>(inst.op));
    break;
  }
}

void CPU::execute_0x9(const instruction_t& inst) {
  const auto last = inst.op & 0x0f;
  if (last == 0) {
    // NOP
    return;
  }
  if (last < 8) {
    // last == regnum (0-7);
    auto left = r16(last);
    auto right = r16(0);
    swap(left, right);
    return;
  } 
  switch (inst.op & 0x0f) {
  // 98: CBW: AX = sign-extend of AL.
  case 0x8: core.regs.h.ah = (core.regs.h.al & 0x80) ? 0xff : 0x00; break;
  // 99: CWD: DX:AX = sign-extend of AX.
  case 0x9: core.regs.x.dx = (core.regs.x.ax & 0x8000) ? 0xffff : 0x0000; break;
  // 9C: Push lower 16 bits of EFLAGS.
  case 0xC: push(core.flags.value_); break;
  // 9D: Pop top of stack into lower 16 bits of EFLAGS.
  case 0xD: core.flags.value_ = pop(); break;
  default:
    LOG(WARNING) << fmt::format("Skipped OPCODE: 0x{:02x}", static_cast<int>(inst.op));
    break;
  }
}

void CPU::scas_m8(const instruction_t& inst) {
  const int16_t step = core.flags.dflag() ? -1 : 1;
  const auto b = memory.get<uint8_t>(core.sregs.es, core.regs.x.di);
  auto r = r8(&core.regs.h.al);
  r.cmp(b);
  VLOG(4) << "SCAS: left: " << static_cast<int>(r.get()) << "; right: " << static_cast<int>(b)
          << "; ZF: " << core.flags.zflag();
  core.regs.x.di += step;
}

void CPU::scas_m16(const instruction_t& inst) {
  const int16_t step = core.flags.dflag() ? -2 : 2;
  const auto b = memory.get<uint16_t>(core.sregs.es, core.regs.x.di);
  auto r = r16(&core.regs.x.ax);
  r.cmp(b);
  VLOG(4) << "SCAS: left: " << static_cast<int>(r.get()) << "; right: " << static_cast<int>(b)
          << "; ZF: " << core.flags.zflag();
  core.regs.x.di += step;
}

void CPU::execute_0xA(const instruction_t& inst) {
  switch (inst.op & 0x0f) {
  case 0x0: {
    const auto seg = core.sregs.get(inst.seg_index());
    // offset is inst.imm8
    core.regs.h.al = memory.get<uint8_t>(seg, inst.imm8);
  } break;
  case 0x1: {
    const auto seg = core.sregs.get(inst.seg_index());
    // offset is inst.imm16
    core.regs.x.ax = memory.get<uint16_t>(seg, inst.imm16);
  } break;
  case 0x2: {
    const auto seg = core.sregs.get(inst.seg_index());
    // offset is inst.imm8
    memory.set<uint8_t>(seg, inst.imm8, core.regs.h.al);
  } break;
  //  mov [seg:off],ax
  case 0x3: {
    const auto seg = core.sregs.get(inst.seg_index());
    // offset is inst.imm16
    memory.set<uint16_t>(seg, inst.imm16, core.regs.x.ax);
  } break;
  // MOVS m8, m8
  case 0x4: {
    const int16_t step = core.flags.dflag() ? -1 : 1;
    const auto src = memory.get<uint8_t>(core.sregs.ds, core.regs.x.si);
    memory.set<uint8_t>(core.sregs.es, core.regs.x.di, src);
    core.regs.x.si += step;
    core.regs.x.di += step;
  } break;
  // MOVS m16, m16
  case 0x5: {
    const int16_t step = core.flags.dflag() ? -2 : 2;
    const auto src = memory.get<uint16_t>(core.sregs.ds, core.regs.x.si);
    memory.set<uint16_t>(core.sregs.es, core.regs.x.di, src);
    core.regs.x.si += step;
    core.regs.x.di += step;
  } break;
  // CMPS m8, m8
  case 0x6: {
    const int16_t step = core.flags.dflag() ? -1 : 1;
    auto left = mem16(core.sregs.ds, core.regs.x.si);
    const auto right = mem16(core.sregs.es, core.regs.x.di);
    left.cmp(right);
    core.regs.x.di += step;
  } break;
  // CMPS m16, m16
  case 0x7: {
    const int16_t step = core.flags.dflag() ? -2 : 2;
    auto left = mem16(core.sregs.ds, core.regs.x.si);
    const auto right = mem16(core.sregs.es, core.regs.x.di);
    left.cmp(right);
    core.regs.x.di += step;
  } break;
  // A8 ib: TEST AL, imm8
  case 0x8: {
    uint8_t imm{inst.imm8};
    Rmm<RmmType::REGISTER, uint8_t> op(&core, &imm);
    auto r = r8(&core.regs.h.al);
    op &= r;
  } break;
  // A9 iw: TEST AX, imm16
  case 0x9: {
    uint16_t imm{inst.imm16};
    Rmm<RmmType::REGISTER, uint16_t> op(&core, &imm);
    auto r = r16(&core.regs.x.ax);
    op &= r;
  } break;
  // STOS m8
  case 0xA: {
    const int16_t step = core.flags.dflag() ? -1 : 1;
    memory.set<uint8_t>(core.sregs.es, core.regs.x.di, core.regs.h.al);
    core.regs.x.di += step;
  } break;
  // STOS m16
  case 0xB: {
    const int16_t step = core.flags.dflag() ? -2 : 2;
    memory.set<uint16_t>(core.sregs.es, core.regs.x.di, core.regs.x.ax);
    core.regs.x.di += step;
  } break;
  // LODSB
  case 0xC: {
    const int16_t step = core.flags.dflag() ? -1 : 1;
    auto r = r8(&core.regs.h.al);
    r.set(memory.get<uint8_t>(core.sregs.ds, core.regs.x.si));
    core.regs.x.si += step;
  } break;
  // LODSW
  case 0xD: {
    const int16_t step = core.flags.dflag() ? -2 : 2;
    auto r = r16(&core.regs.x.ax);
    r.set(memory.get<uint16_t>(core.sregs.ds, core.regs.x.si));
    core.regs.x.si += step;
  } break;
  // SCAS m8
  case 0xE: scas_m8(inst); break;
  // SCAS m16
  case 0xF: scas_m16(inst); break;
  default: LOG(WARNING) << "Skipped OPCODE: " << std::hex << inst.op; break;
  }
}

void CPU::execute_0xB(const instruction_t& inst) {
  const auto wide = inst.op & 0x08;
  const auto regnum = inst.op & 0x07;
  if (wide) {
    core.regs.x.set(regnum, inst.imm16);
  } else {
    core.regs.h.set(regnum, inst.imm8);
  }
}

void CPU::execute_0xC(const instruction_t& inst) {
  switch (inst.op & 0x0f) {
  case 0x0: return execute_0xC0(inst, inst.mdrm.reg);
  case 0x1: return execute_0xC1(inst, inst.mdrm.reg);
  // RET (near call and clear some bytes of stack)
  case 0x2: {
    core.ip = pop();
    VLOG(1) << fmt::format("RET and clear stack: num: {:02x}; SP: {:02x}", inst.imm16,
                           core.regs.x.sp);
    core.regs.x.sp += inst.imm16;
  } break;
  // RET (near call)
  case 0x3: {
    core.ip = pop();
  } break;
  // LES r16,m16:16 - Load ES:r16 with far pointer from memory.
  case 0x4: {
    const auto seg = core.sregs.get(inst.seg_index());
    const auto mem_off = memory.get<uint16_t>(seg, inst.disp16);
    const auto mem_seg = memory.get<uint16_t>(seg, inst.disp16 + 2);
    core.regs.x.set(inst.mdrm.reg, mem_off);
    core.sregs.es = mem_seg;
  } break;
  // LDS r16,m16:16 - Load DS:r16 with far pointer from memory.
  // 
  // For example, the instruction LDS SI, [200h] is equivalent to the pair of
  // instructions:
  // MOV SI, [200h] and MOV DS, [202h]. The 8086 only supports loading the
  // pointer into the DS or ES segment register.
  case 0x5: {
    const auto seg = core.sregs.get(inst.seg_index());
    core.regs.x.set(inst.mdrm.reg, memory.get<uint16_t>(seg, inst.disp16));
    core.sregs.ds = memory.get<uint16_t>(seg, inst.disp16 + 2);
  } break;
  // MOV r/m8, imm8
  case 0x6: {
    auto rm = rmm8(inst);
    rm.set(inst.disp8);
  } break;
  // MOV r/m16, imm16
  case 0x7: {
    auto rm = rmm16(inst);
    rm.set(inst.disp16);
  } break;
  // RET (far call and clear some bytes of stack)
  case 0xA: {
    core.ip = pop();
    core.sregs.cs = pop();
    VLOG(1) << fmt::format("RETF and clear stack: num: {:02x}; SP: {:02x}", inst.disp16,
                           core.regs.x.sp);
    core.regs.x.sp += inst.disp16;
  } break;
  // RET (far call)
  case 0xB: {
    core.ip = pop();
    core.sregs.cs = pop();
  } break;
  // "CD":"int imm8",
  case 0xC: call_interrupt(0x03); break;
  case 0xD: call_interrupt(inst.imm8); break;
  default: LOG(WARNING) << fmt::format("Skipped OPCODE: 0x{:02x}", static_cast<int>(inst.op));
  }
}

void CPU::execute_0xC0(const instruction_t& inst, int subop) {
  auto r = rmm8(inst);
  switch (subop) {
  case 0: r.rol(inst.disp8); break;
  case 1: r.ror(inst.disp8); break;
  case 2: r.rcl(inst.disp8); break;
  case 3: r.rcr(inst.disp8); break;
  case 4: r.shl(inst.disp8); break;
  case 5: r.shr(inst.disp8); break;
  case 6: r.shl(inst.disp8); break;
  case 7: r.sar(inst.disp8); break;
  }
}

void CPU::execute_0xC1(const instruction_t& inst, int subop) {
  auto r = rmm16(inst);
  switch (subop) {
  case 0: r.rol(inst.disp8); break;
  case 1: r.ror(inst.disp8); break;
  case 2: r.rcl(inst.disp8); break;
  case 3: r.rcr(inst.disp8); break;
  case 4: r.shl(inst.disp8); break;
  case 5: r.shr(inst.disp8); break;
  case 6: r.shl(inst.disp8); break;
  case 7: r.sar(inst.disp8); break;
  }
}

void CPU::execute_0xD0(const instruction_t& inst, int subop) {
  auto r = rmm8(inst);
  switch (subop) {
  case 0: r.rol(1); break;
  case 1: r.ror(1); break;
  case 2: r.rcl(1); break;
  case 3: r.rcr(1); break;
  case 4: r.shl(1); break;
  case 5: r.shr(1); break;
  // undocumented SHL/SAL
  case 6: r.shl(1); break;
  case 7: r.sar(1); break;
  }
}

void CPU::execute_0xD1(const instruction_t& inst, int subop) {
  auto r = rmm16(inst);
  switch (subop) {
  case 0: r.rol(1); break;
  case 1: r.ror(1); break;
  case 2: r.rcl(1); break;
  case 3: r.rcr(1); break;
  case 4: r.shl(1); break;
  case 5: r.shr(1); break;
  // undocumented SHL/SAL
  case 6: r.shl(1); break;
  case 7: r.sar(1); break;
  }
}

void CPU::execute_0xD2(const instruction_t& inst, int subop) {
  auto r = rmm8(inst);
  switch (subop) {
  case 0: r.rol(core.regs.h.cl); break;
  case 1: r.ror(core.regs.h.cl); break;
  case 2: r.rcl(core.regs.h.cl); break;
  case 3: r.rcr(core.regs.h.cl); break;
  case 4: r.shl(core.regs.h.cl); break;
  case 5: r.shr(core.regs.h.cl); break;
  // undocumented SHL/SAL
  case 6: r.shl(core.regs.h.cl); break;
  case 7: r.sar(core.regs.h.cl); break;
  }
}

void CPU::execute_0xD3(const instruction_t& inst, int subop) {
  auto r = rmm16(inst);
  switch (subop) {
  case 0: r.rol(core.regs.h.cl); break;
  case 1: r.ror(core.regs.h.cl); break;
  case 2: r.rcl(core.regs.h.cl); break;
  case 3: r.rcr(core.regs.h.cl); break;
  case 4: r.shl(core.regs.h.cl); break;
  case 5: r.shr(core.regs.h.cl); break;
  // undocumented SHL/SAL
  case 6: r.shl(core.regs.h.cl); break;
  case 7: r.sar(core.regs.h.cl); break;
  }
}

void CPU::execute_0xD(const instruction_t& inst) {
  switch (inst.op & 0x0f) {
  case 0x0: execute_0xD0(inst, inst.mdrm.reg); break;
  case 0x1: execute_0xD1(inst, inst.mdrm.reg); break;
  case 0x2: execute_0xD2(inst, inst.mdrm.reg); break;
  case 0x3: execute_0xD3(inst, inst.mdrm.reg); break;
  }
}

void CPU::execute_0xE(const instruction_t& inst) {
  switch (inst.op & 0x0f) {
  // LOOPNE rel8
  case 0x0: {
    if (core.regs.x.cx-- == 0 && !core.flags.zflag()) {
      core.ip += static_cast<int8_t>(inst.imm8);
    }
  } break;
  // LOOPE rel8
  case 0x1: {
    if (core.regs.x.cx-- == 0 && core.flags.zflag()) {
      core.ip += static_cast<int8_t>(inst.imm8);
    }
  } break;
  // LOOP rel8
  case 0x2: {
    if (core.regs.x.cx-- == 0) {
      core.ip += static_cast<int8_t>(inst.imm8);
    }
  } break;
  // Jump short if eCX register is 0
  case 0x3: {
    if (core.regs.x.cx == 0) {
      core.ip += static_cast<int8_t>(inst.imm8);
    }
  } break;
  // IN AL, imm8
  case 0x4: core.regs.h.al = io.inb(inst.imm8); break;
  // IN AX, imm8
  case 0x5: core.regs.x.ax = io.inw(inst.imm8); break;
  // OUT AL, imm8
  case 0x6: io.outb(inst.imm8, core.regs.h.al); break;
  // OUT AX, imm8
  case 0x7: io.outw(inst.imm8, core.regs.x.ax); break;
  // CALL rel16
  case 0x8: {
    // push the next IP onto the stack then jump ahead by the specified offset.
    push(core.ip);
    core.ip += static_cast<int16_t>(inst.imm16);
  } break;
  // JMP rel16
  case 0x9: core.ip += static_cast<int16_t>(inst.imm16); break;
  // JMP rel8
  case 0xB: core.ip += static_cast<int8_t>(inst.imm8); break;
  // IN AL, DX
  case 0xC: core.regs.h.al = io.inb(core.regs.x.dx); break;
  // IN AX, DX
  case 0xD: core.regs.x.ax = io.inw(core.regs.x.dx); break;
  // OUT AL, DX
  case 0xE: io.outb(core.regs.x.dx, core.regs.h.al); break;
  // OUT AX, DX
  case 0xF: io.outw(core.regs.x.dx, core.regs.x.ax); break;
  default: LOG(WARNING) << fmt::format("Skipped OPCODE: 0x{:02x}", static_cast<int>(inst.op));
  }
}

void CPU::execute_0xF6(const instruction_t& inst, int subop) {
  switch (inst.mdrm.reg) {
  case 0: {
    LOG(WARNING) << "Need SOME WAY TO ENCODE THIS! F6/0 with imm8";
    uint8_t imm{inst.imm8};
    Rmm<RmmType::REGISTER, uint8_t> op(&core, &imm);
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
  // F6 /4 MUL r/m8
  case 0x4: {
    core.regs.x.ax = core.regs.h.al * inst.disp8;
    core.flags.oflag(core.regs.h.ah);
    core.flags.cflag(core.regs.h.ah);
  } break;
  // F6 /5 IMUL r/m8
  case 0x5: {
    core.regs.x.ax = core.regs.h.al * inst.disp8;
    core.flags.oflag((core.regs.h.al & 0x80) && core.regs.h.ah);
    core.flags.cflag((core.regs.h.al & 0x80) && core.regs.h.ah);
  } break;
  }
}

void CPU::execute_0xF7(const instruction_t& inst, int subop) {
  switch (inst.mdrm.reg) {
  case 0: {
    LOG(WARNING) << "Need SOME WAY TO ENCODE THIS! F7/0 with imm16";
    uint16_t operand{inst.disp16};
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
    uint32_t result = core.regs.x.ax * inst.disp16;
    core.regs.x.dx = ((result & 0xFF00) >> 16);
    core.flags.oflag((core.regs.x.ax & 0x8000) && core.regs.x.dx);
    core.flags.cflag((core.regs.x.ax & 0x8000) && core.regs.x.dx);
  } break;
  // F7 /5 IMUL r/m16
  case 0x5: {
    uint32_t result = core.regs.x.ax * inst.disp16;
    core.regs.x.dx = ((result & 0xFF00) >> 16);
    core.flags.oflag(core.regs.x.dx);
    core.flags.cflag(core.regs.x.dx);
  } break;
  }
}

void CPU::execute_0xFE(const instruction_t& inst, int subop) {
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
    LOG(WARNING) << "Unhandled subcode of opcode: 0xFE; subcode: "
                 << static_cast<int>(inst.mdrm.reg);
  } break;
  }
}

void CPU::execute_0xFF(const instruction_t& inst, int subop) {
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
    auto rm = rmm16(inst);
    core.ip = rm.get();
  } break;
  // JMP r/m16
  case 4: {
    auto rm = rmm16(inst);
    core.ip = rm.get();
  } break;
  // 5: JMP m16:16
  case 5: {
    // TODO(rushfan): Need an example of this. - This may not be right.
    const auto seg = core.sregs.get(inst.seg_index());
    core.ip = memory.get<uint16_t>(seg, inst.disp16);
    core.sregs.cs = memory.get<uint16_t>(seg, inst.disp16 + 2);
  } break;
  case 6: {
    const auto r = rmm16(inst);
    push(r.get());
  } break;
  default: {
    LOG(WARNING) << "Unhandled subcode of opcode: 0xFF; subcode: "
                 << static_cast<int>(inst.mdrm.reg);
  } break;
  }
}


void CPU::execute_0xF(const instruction_t& inst) {
  switch (inst.op & 0x0f) {
  // 0xF5: CMC
  case 0x5: core.flags.cflag(!core.flags.cflag()); break;
  // 0xF6/M
  case 0x6: execute_0xF6(inst, inst.mdrm.reg); break;
  // 0xF7/M
  case 0x7: execute_0xF7(inst, inst.mdrm.reg); break;
  // CLC: Clear carry flag
  case 0x8: core.flags.cflag(false); break;
  // STC: Set carry flag
  case 0x9: core.flags.cflag(true); break;
  // CLI: Clear interrupt flag
  case 0xA: core.flags.iflag(false); break;
  // STI: Set interrupt flag
  case 0xB: core.flags.iflag(true); break;
  // CLD: Clear directuon flag
  case 0xC: core.flags.dflag(false); break;
  // STD—Set Direction Flag
  case 0xD: core.flags.dflag(true); break;
  // INC or DEC r/m8
  case 0xE: execute_0xFE(inst, inst.mdrm.reg); break;
  // INC r/m16
  case 0xF: execute_0xFF(inst, inst.mdrm.reg); break;
  default: LOG(WARNING) << fmt::format("Skipped OPCODE: 0x{:02x}", static_cast<int>(inst.op));
  }
}

void CPU::call_interrupt(int num) {
  if (auto fn = int_handlers_.find(num); fn != std::end(int_handlers_)) {
    fn->second(num, *this);
    return;
  }
  // static default fail safe handlers.
  if (num >= 0x86 && num <= 0xF0) {
    // INT 86 to F0: INT 86 to F0 - used by BASIC while in interpreter
    running_ = false;
    LOG(INFO) << "Exiting; Out of band Interrupt Num: 0x" << std::hex << num;
  } else {
    LOG(INFO) << fmt::format("Unhandled Interrupt Num: 0x{:02X} {:04X}:{:04X}", num, core.regs.h.ah,
                             core.regs.h.al);
  }
}

bool CPU::run(uint16_t start_cs, uint16_t start_ip) {
  core.sregs.cs = start_cs;
  core.ip = start_ip;
  return run();
}

bool CPU::run() {
  while (running_) {
    const int pos = (core.sregs.cs * 0x10) + core.ip;
    const auto inst = decoder.decode(&memory[pos]);
    if (VLOG_IS_ON(3)) {
      const auto line =
          fmt::format("[{:04x}:{:04x}] inst: {}", core.sregs.cs, core.ip, inst.DebugString());
      VLOG(3) << line;
    }
    core.ip += inst.len;
    execute(inst);
    if (VLOG_IS_ON(4)) {
      VLOG(4) << core.DebugString();
      std::cerr << std::endl;
    }
  }
  return true;
}

bool CPU::execute(const instruction_t& inst) {
  if (inst.metadata.mask & op_mask_notimpl) {
    LOG(WARNING) << fmt::format("Unimplemented Opcode Encountered: {:02X} at IP: {:02X}",
      static_cast<uint16_t>(inst.op), core.ip - 1);
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
      if (inst.metadata.mask & uses_rep_zf) {
        done = core.flags.zflag();    
      }
    } else if (inst.repne) {
      if (--core.regs.x.cx == 0) {
        done = true;
        break;
      }
      if (inst.metadata.mask & uses_rep_zf) {
        done = !core.flags.zflag();
      }
    }
  } while (!done);
  return true;
}


void CPU::push(uint16_t val) { 
  // To push we decrement the stack pointer and then add it.
  core.regs.x.sp -= 2;
  // TODO(rushfan): assert if sp <= 0x0000
  VLOG(1) << fmt::format("PUSH: val: {:02x}; SP: {:02x}", val, core.regs.x.sp);
  memory.set<uint16_t>(core.sregs.ss, core.regs.x.sp, val);
}

uint16_t CPU::pop() {
  const auto m = memory.get<uint16_t>(core.sregs.ss, core.regs.x.sp);
  core.regs.x.sp += 2; 
  // TODO(rushfan): assert if sp >= 0xffff
  VLOG(1) << fmt::format("POP: val: {:02x}; SP: {:02x}", m, core.regs.x.sp);
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
    return Rmm<RmmType::EITHER, uint8_t>(&core, &memory, seg, inst.disp16);
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
    // Let's just go with modrm16 is always 16bit
    return Rmm<RmmType::EITHER, uint16_t>(&core, &memory, seg, inst.disp16);
  }
  const auto offset = effective_address(inst, core);
  return Rmm<RmmType::EITHER, uint16_t>(&core, &memory, seg, offset);
}


// returns the register for an r16
Rmm<RmmType::REGISTER, uint8_t> CPU::r8(const instruction_t& inst) {
  return Rmm<RmmType::REGISTER, uint8_t>(&core, core.regs.h.regptr(inst.mdrm.reg));
}

Rmm<RmmType::REGISTER, uint16_t> CPU::r16(int regnum) {
  return Rmm<RmmType::REGISTER, uint16_t>(&core, core.regs.x.regptr(regnum));
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

Rmm<RmmType::MEMORY, uint8_t> CPU::mem8(uint16_t seg, uint16_t offset) {
  return Rmm<RmmType::MEMORY, uint8_t>(&core, &memory, seg, offset);
}

Rmm<RmmType::MEMORY, uint16_t> CPU::mem16(uint16_t seg, uint16_t offset) {
  return Rmm<RmmType::MEMORY, uint16_t>(&core, &memory, seg, offset);
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

