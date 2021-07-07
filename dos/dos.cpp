#include "dos/dos.h"

#include "core/file.h"
#include "core/log.h"
#include "core/scope_exit.h"
#include "dos/exe.h"
#include "fmt/format.h"
#include "fmt/printf.h"
#include <cstdio>
#include <filesystem>
#include <optional>
#include <string>

// MSVC only has __PRETTY_FUNCTION__ in intellisense,
// TODO(rushfan): Find a better home for this macro.
#if !defined(__PRETTY_FUNCTION__)
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

using namespace wwiv::core;

namespace door86::dos {

bool Dos::initialize_process(const std::filesystem::path& filename) {
  if (psp_) {
    LOG(WARNING) << "Trying to reinitialize a process with existing PSP";
    return false;
  }

  int code_offset = 0x100;
  int memory_needed = 0xffff;

  const std::string env = R"(COMSPEC=Z:\DOS\COMMAND.COM\0)";
  const auto env_needed = ((env.size() | 0x3F) + 1) * 2;
  auto eseg = mem_mgr.allocate(env_needed);
  if (!eseg) {
    std::cout << "Failed to allocate memory: " << memory_needed;
    return EXIT_FAILURE;
  }
  LOG(INFO) << fmt::format("ENV SEG:  {:04X} ", eseg.value());

  if (is_exe(filename)) {
    auto o = read_exe_header(filename);
    if (!o) {
      std::cout << "Failed to read exe header for file: " << filename;
      return EXIT_FAILURE;
    }
    auto exe = o.value();
    code_offset = exe.header_size();
    memory_needed = exe.memory_needed();
    auto oseg = mem_mgr.allocate(memory_needed);
    if (!oseg) {
      std::cout << "Failed to allocate memory: " << memory_needed;
      return EXIT_FAILURE;
    }
    const auto psp_seg = oseg.value();
    exe.load_image(psp_seg, cpu_->memory);
    cpu_->core.sregs.ds = psp_seg;
    cpu_->core.sregs.es = psp_seg;

    // skip PSP
    const auto seg = psp_seg + 0x10;
    LOG(INFO) << fmt::format("PSP SEG:  {:04X} ", psp_seg);
    cpu_->core.sregs.cs = exe.hdr.cs + seg;
    cpu_->core.sregs.ss = exe.hdr.ss + seg;
    cpu_->core.regs.x.sp = exe.hdr.sp + seg;
    cpu_->core.ip = exe.hdr.ip;
    LOG(INFO) << "CS: " << cpu_->core.sregs.cs;
    // load_image will relocate relos
  } else {
    //
    // COM file
    //
    auto oseg = mem_mgr.allocate(memory_needed);
    if (!oseg) {
      std::cout << "Failed to allocate memory: " << memory_needed;
      return EXIT_FAILURE;
    }
    const auto psp_seg = oseg.value();
    cpu_->core.sregs.ds = psp_seg;
    cpu_->core.sregs.es = psp_seg;

    const auto seg = psp_seg + 0x100;
    cpu_->core.sregs.cs = seg;
    cpu_->core.sregs.ss = seg;
    cpu_->core.regs.x.sp = seg;
    cpu_->core.ip = 0;
  }

  void* m = &cpu_->memory[cpu_->core.sregs.ds * 0x10];
  psp_ = std::make_unique<PSP>(m);
  psp_->initialize();
  psp_->psp->environ_seg = eseg.value();

  return true;
}


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

void Dos::int20(int, door86::cpu::x86::CPU& cpu) { cpu_->halt(); }

void Dos::int21(int, door86::cpu::x86::CPU& cpu) {
  LOG(INFO) << fmt::format("[{:04x}:{:04x}] DOS Interrupt: 0x{:04x}; {:02X}", cpu_->core.sregs.cs,
                           cpu_->core.ip, cpu_->core.regs.x.ax, static_cast<int>(cpu_->core.regs.h.ah));
  switch (cpu_->core.regs.h.ah) {
  // terminate app
  case 0x00: cpu_->halt(); break;
  // read char
  case 0x01: {
    // TODO(rushfan): Need to do break checking, etc.
    cpu_->core.regs.h.al = static_cast<uint8_t>(fgetc(stdin));
  } break;
  // display char
  case 0x02: fputc(cpu_->core.regs.h.dl, stdout); break;
  // display string
  case 0x9: {
    for (auto offset = cpu_->core.regs.x.dx;; ++offset) {
      const auto m = cpu_->memory.get<uint8_t>(cpu_->core.sregs.ds, offset);
      if (m == '$' || m == '\0') {
        // TODO(rushfan): We shouldn't stop at \0, but we will for now.
        break;
      }
      fputc(m, stdout);
    }
  } break;
  // INT 21 - AH = 25h DOS - SET INTERRUPT VECTOR
  case 0x25: {
    const auto s = fmt::format("{:04X}:{:04X}", cpu_->core.sregs.ds, cpu_->core.regs.x.dx);
    LOG(INFO) << "Set Dos Interrupt for: " << static_cast<int>(cpu_->core.regs.h.al) << "; " << s;
  } break;
  // INT 21 - DOS 2+ - GET DOS VERSION
  case 0x30:
    VLOG(2) << "Get DOS Version";
    cpu_->core.regs.x.ax = 0x0005;
    cpu_->core.regs.x.bx = 0x0000;
    cpu_->core.regs.x.cx = 0x0000;
    break;
  // Get Interrupt Vector
  case 0x35: {
    VLOG(2) << "Get Interrupt Vector for: " << fmt::format("{:02X}", cpu_->core.regs.h.al);
    // TODO(rushfan): HACK
    cpu_->memory[(cpu_->core.sregs.es * 0x10) + cpu_->core.regs.x.bx] = cpu_->core.regs.h.al;
    // segment is 0
    cpu_->memory[(cpu_->core.sregs.es * 0x10) + cpu_->core.regs.x.bx + 2] = 0;
  } break;
  // terminate app.
  case 0x4c: 
    VLOG(2) << "Terminate App";
    cpu_->halt(); 
    break;
  default: {
    // unhandled
    VLOG(2) << "Unhandled DOS Interrupt "<< fmt::format("AH: {:02X}; AL: {:02X}", cpu_->core.regs.h.ah, cpu_->core.regs.h.al);
  } break;
  }
}


}

