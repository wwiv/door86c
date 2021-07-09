#include "cpu/x86/rmm.h"
#include "cpu/x86/cpu.h"

#include "core/log.h"
#include "fmt/format.h"
#include <iomanip>
#include <iostream>

namespace door86::cpu::x86 {

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
  case 0: {
    if (inst.mdrm.rm == 0x06) {
      return inst.disp16;
    }
    return base_ea(inst.mdrm, core);
  }
  case 1: return base_ea(inst.mdrm, core) + inst.disp8;
  case 2: return base_ea(inst.mdrm, core) + inst.disp16;
  case 3: LOG(FATAL) << "Whoops! Getting Effective Address for mod3??!?!?"; return 0;
  default: LOG(FATAL) << "Whoops! Unknown mod!" << inst.mdrm.mod; return 0;
  }
}

} // namespace door86::cpu::x86
