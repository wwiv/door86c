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
  cpu_->int_handlers().try_emplace(
      0x20, std::bind(&Dos::int20, this, std::placeholders::_1, std::placeholders::_2));
  cpu_->int_handlers().try_emplace(
      0x21, std::bind(&Dos::int21, this, std::placeholders::_1, std::placeholders::_2));

  // Setup vector pointing to our bogus locations.
  for (auto i = 0; i < 0xff; i++) {
    // offset i, segment 0;
    cpu_->memory[i * 4] = i;
  }
}

void Dos::int20(int, door86::cpu::x86::CPU& cpu) { cpu.halt(); }

void Dos::int21(int, door86::cpu::x86::CPU& cpu) {
  LOG(INFO) << fmt::format("[{:04x}:{:04x}] DOS Interrupt: 0x{:04x}; {:02X}", cpu.core.sregs.cs,
                           cpu.core.ip, cpu.core.regs.x.ax, static_cast<int>(cpu.core.regs.h.ah));
  switch (cpu.core.regs.h.ah) {
  // terminate app
  case 0x00: cpu.halt(); break;
  // read char
  case 0x01: {
    // TODO(rushfan): Need to do break checking, etc.
    cpu.core.regs.h.al = static_cast<uint8_t>(fgetc(stdin));
  } break;
  // display char
  case 0x02: fputc(cpu.core.regs.h.dl, stdout); break;
  // display string
  case 0x9: {
    for (auto offset = cpu.core.regs.x.dx;; ++offset) {
      const auto m = cpu.memory.get<uint8_t>(cpu.core.sregs.ds, offset);
      if (m == '$' || m == '\0') {
        // TODO(rushfan): We shouldn't stop at \0, but we will for now.
        break;
      }
      fputc(m, stdout);
    }
  } break;
  // INT 21 - AH = 25h DOS - SET INTERRUPT VECTOR
  case 0x25: {
    const auto s = fmt::format("{:04X}:{:04X}", cpu.core.sregs.ds, cpu.core.regs.x.dx);
    LOG(INFO) << "Set Dos Interrupt for: " << static_cast<int>(cpu.core.regs.h.al) << "; " << s;
  } break;
  // INT 21 - DOS 2+ - GET DOS VERSION
  case 0x30:
    VLOG(2) << "Get DOS Version";
    cpu.core.regs.x.ax = 0x0050;
    cpu.core.regs.x.bx = 0x0000;
    cpu.core.regs.x.cx = 0x0000;
    break;
  // Get Interrupt Vector
  case 0x35: {
    VLOG(2) << "Get Interrupt Vector for: " << fmt::format("{:02X}", cpu.core.regs.h.al);
    // TODO(rushfan): HACK
    cpu.core.sregs.es = 0x0020;
    cpu.core.regs.x.bx = cpu.core.regs.h.al;
  } break;
  // terminate app.
  case 0x4c: 
    VLOG(2) << "Terminate App";
    cpu.halt(); 
    break;
  default: {
    // unhandled
    VLOG(2) << "Unhandled DOS Interrupt "<< fmt::format("AH: {:02X}; AL: {:02X}", cpu.core.regs.h.ah, cpu.core.regs.h.al);
  } break;
  }
}


}

