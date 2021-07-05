#include "dos/dos.h"

#include "core/file.h"
#include "core/log.h"
#include "core/scope_exit.h"
#include "fmt/format.h"
#include "fmt/printf.h"
#include <cstdio>
#include <optional>
#include <string>

// MSVC only has __PRETTY_FUNCTION__ in intellisense,
// TODO(rushfan): Find a better home for this macro.
#if !defined(__PRETTY_FUNCTION__)
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

namespace door86::dos {

using namespace wwiv::core;

// allocate a block of memory of size bytes, returns the starting segment;
std::optional<uint16_t> DosMemoryManager::allocate(size_t size) { 
  const auto segs_needed = static_cast<uint16_t>(1 + (size / 16));
  if ((end_seg_ - start_seg_) < segs_needed) {
    // not enough memory.
    return std::nullopt;
  }
  auto seg = top_seg_;
  // to start with we'll load from the bottom
  top_seg_ += segs_needed;
  return {seg};
}

void DosMemoryManager::free(uint16_t seg) {
  //TODO(rushfan): Implement free
}


Dos::Dos(door86::cpu::x86::CPU* cpu) : cpu_(cpu) {
  cpu_->int_handlers() .try_emplace(0x21, std::bind(&Dos::int21, this, std::placeholders::_1, std::placeholders::_2));
}

void Dos::int21(int, door86::cpu::x86::CPU& cpu) {
  fmt::print("DOS Interrupt: 0x{:04x}\r\n", cpu.core.regs.x.ax);
  switch (cpu.core.regs.h.ah) {
  // display string
  case 0x09: {
    for (auto offset = cpu.core.regs.x.dx;; ++offset) {
      const auto m = cpu.memory.get<uint8_t>(cpu.core.sregs.ds, offset);
      if (m == '$' || m == '\0') {
        // TODO(rushfan): We shouldn't stop at \0, but we will for now.
        break;
      }
      fputc(m, stdout);
    }
  } break;
  case 0x4c: // terminate app.
    cpu.halt();
    break;
  default: {
    // unhandled
  } break;
  }
}


}

