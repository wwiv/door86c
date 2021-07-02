#include "cpu/x86/rmm.h"
#include "cpu/x86/core.h"

#include "core/log.h"
#include "fmt/format.h"
#include <iomanip>
#include <iostream>

namespace door86::cpu::x86 {
//
// returns the register for an r16
uint16_t r16(const instruction_t& inst, cpu_core& core) {
  if (inst.metadata.mask & op_mask_reg_is_sreg) {
    return core.sregs.get(inst.mdrm.reg);
  }
  return core.regs.x.get(inst.mdrm.reg);
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
  if (inst.mdrm.mod == 0x03) {
    // mod 3 returns a reference to a CPU register
    return Rmm<uint8_t>(&core, core.regs.h.regptr(inst.mdrm.rm));
  }
  // Pick the overridden segment, or just the default one for the instruction.
  const auto seg_index =
      inst.seg_override.value_or(default_segment_for_index(inst.mdrm.mod, inst.mdrm.rm));
  const uint16_t seg = core.sregs.get(seg_index);
  if (inst.mdrm.mod == 0 && inst.mdrm.rm == 0x06) {
    const auto offset =
        inst.metadata.bits == 8 ? inst.operand8 : static_cast<uint8_t>(inst.operand16 & 0xff);
    return Rmm<uint8_t>(&core, &mem, seg, offset);
  }
  const auto offset = effective_address(inst, core);
  return Rmm<uint8_t>(&core, &mem, seg, offset);
}

Rmm<uint16_t> rmm16(const instruction_t& inst, cpu_core& core, Memory& mem) {
  if (inst.mdrm.mod == 0x03) {
    // mod 3 returns a reference to a CPU register.
    // don't ue r16 since that applies segment override, and ModR/M bytes
    // in mod 3 don't ever use segment registers.
    return Rmm<uint16_t>(&core, core.regs.x.regptr(inst.mdrm.rm));
  }
  const segment_t senum =
      inst.seg_override.value_or(default_segment_for_index(inst.mdrm.mod, inst.mdrm.rm));
  uint16_t seg = core.sregs.get(senum);
  if (inst.mdrm.mod == 0 && inst.mdrm.rm == 0x06) {
    const auto offset = inst.metadata.bits == 8 ? inst.operand8 : inst.operand16;
    return Rmm<uint16_t>(&core, &mem, seg, offset);
  }
  const auto offset = effective_address(inst, core);
  return Rmm<uint16_t>(&core, &mem, seg, offset);
}

}