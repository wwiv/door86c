#ifndef INCLUDED_CPU_X86_CPU_CORE_H
#define INCLUDED_CPU_X86_CPU_CORE_H

#include "cpu/memory.h"
#include "cpu/x86/decoder.h"
#include "cpu/x86/regs.h"
#include <cstdint>
#include <string>

// Start with instructons needed for hello world in asm, then expand
// to these, then on to others as needed.
// https:/a/github.com/xem/minix86/blob/gh-pages/src/instructions.js

namespace door86::cpu::x86 {

class cpu_core {
public:
  regs_t regs;
  sregs_t sregs;
  flags_t flags;
  uint16_t ip{0};
  std::string DebugString() const;
};

} // namespace door86::cpu::x86

#endif