#include "cpu/x86/rmm.h"
#include "cpu/x86/cpu.h"

#include "core/log.h"
#include "fmt/format.h"
#include <iomanip>
#include <iostream>

namespace door86::cpu::x86 {
//
// returns the register for an r16
Rmm<RmmType::REGISTER, uint16_t> r16(const instruction_t& inst, cpu_core& core) {
  if (inst.metadata.mask & op_mask_reg_is_sreg) {
    return Rmm<RmmType::REGISTER, uint16_t>(&core, core.sregs.regptr(inst.mdrm.reg));
  }
  return Rmm<RmmType::REGISTER, uint16_t>(&core, core.regs.x.regptr(inst.mdrm.reg));
}

// returns the register for an r16
Rmm<RmmType::REGISTER, uint8_t> r8(const instruction_t& inst, cpu_core& core) {
  return Rmm<RmmType::REGISTER, uint8_t>(&core, core.regs.h.regptr(inst.mdrm.reg));
}

// base without any displacement.
static uint16_t base_ea(reg_mod_rm modrm, const cpu_core& core) {
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
  switch (inst.mdrm.mod) {
  case 0: return inst.metadata.bits == 8 ? inst.operand8 : inst.operand16;
  case 1: return base_ea(inst.mdrm, core) + static_cast<int8_t>(inst.operand8);
  case 2: return base_ea(inst.mdrm, core) + inst.operand16;
  case 3: LOG(FATAL) << "Whoops! Getting Effective Address for mod3??!?!?"; return 0;
  default: LOG(FATAL) << "Whoops! Unknown mod!" << inst.mdrm.mod; return 0;
  }
}
  // **** NOTE: Keep rmm8 and rmm16 in sync

// returns the offset for the effective address described by a modrm byte
// returns the rm portion for the modrm
Rmm<RmmType::EITHER, uint8_t> rmm8(const instruction_t& inst, cpu_core& core, Memory& mem) {
  if (inst.mdrm.mod == 0x03) {
    // mod 3 returns a reference to a CPU register
    return Rmm<RmmType::EITHER, uint8_t>(&core, core.regs.h.regptr(inst.mdrm.rm));
  }
  // Pick the overridden segment, or just the default one for the instruction.
  const auto seg = core.sregs.get(inst.seg_index());
  if (inst.mdrm.mod == 0 && inst.mdrm.rm == 0x06) {
    const auto offset =
        inst.metadata.bits == 8 ? inst.operand8 : static_cast<uint8_t>(inst.operand16 & 0xff);
    return Rmm<RmmType::EITHER, uint8_t>(&core, &mem, seg, offset);
  }
  const auto offset = effective_address(inst, core);
  return Rmm<RmmType::EITHER, uint8_t>(&core, &mem, seg, offset);
}

Rmm<RmmType::EITHER, uint16_t> rmm16(const instruction_t& inst, cpu_core& core, Memory& mem) {
  if (inst.mdrm.mod == 0x03) {
    // mod 3 returns a reference to a CPU register.
    // don't ue r16 since that applies segment override, and ModR/M bytes
    // in mod 3 don't ever use segment registers.
    return Rmm<RmmType::EITHER, uint16_t>(&core, core.regs.x.regptr(inst.mdrm.rm));
  }
  const auto seg = core.sregs.get(inst.seg_index());
  if (inst.mdrm.mod == 0 && inst.mdrm.rm == 0x06) {
    const auto offset = inst.metadata.bits == 8 ? inst.operand8 : inst.operand16;
    return Rmm<RmmType::EITHER, uint16_t>(&core, &mem, seg, offset);
  }
  const auto offset = effective_address(inst, core);
  return Rmm<RmmType::EITHER, uint16_t>(&core, &mem, seg, offset);
}

}
