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
  auto eseg = mem_mgr.allocate(env_needed, false);
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
    auto oseg = mem_mgr.allocate(memory_needed, true);
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
    cpu_->core.regs.x.sp = exe.hdr.sp;
    cpu_->core.ip = exe.hdr.ip;
    LOG(INFO) << "CS: " << cpu_->core.sregs.cs;
    // load_image will relocate relos
  } else {
    //
    // COM file
    //
    auto oseg = mem_mgr.allocate(memory_needed, true);
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

DosMemoryManager::DosMemoryManager(door86::cpu::Memory* mem, uint16_t start_seg, uint16_t end_seg)
    : mem_(mem_), start_seg_(start_seg), end_seg_(end_seg) {
  // Add the starer block
  memory_block b{};
  b.avail = memory_avail_t::free;
  b.start = start_seg_;
  b.size = (end_seg_ - start_seg_);
  b.owner = 0;
  blocks_.emplace_back(b);
}

memory_block make_block(uint16_t start, uint16_t size, const memory_avail_t avail) {
  memory_block b{};
  b.avail = avail;
  b.size = size;
  b.start = start;
  return b;
}
    // allocate a block of memory of size bytes, returns the starting segment;
std::optional<uint16_t> DosMemoryManager::allocate(size_t size, bool create_mcb) {
  auto segs_needed = static_cast<uint16_t>(size >> 4);
  if (size & 0x000F) {
    ++segs_needed;
  }
  if (create_mcb) {
    ++segs_needed;
  }
  // todo - implement best, jsut use first if not last.
  if (fit_ == fit_strategy_t::last) {
    for (auto& it = std::rbegin(blocks_); it != std::rend(blocks_); ++it) {
      if (it->avail == memory_avail_t::free && it->size >= segs_needed) {
        if (it->size - segs_needed <= 256) {
          // Just make ram used, and return the existing block.
          it->avail = memory_avail_t::used;
          return {it->start};
        }
        // shrink current free block and assign the new one added after it
        it->size -= segs_needed;
        it->avail = memory_avail_t::free;
        const auto start = it->start + it->size;
        blocks_.emplace(it.base(), make_block(start, segs_needed, memory_avail_t::used));
        return {start};
      }
    }
  } else {
    for (auto& it = std::begin(blocks_); it != std::end(blocks_); ++it) {
      if (it->avail == memory_avail_t::free && it->size >= segs_needed) {
        if (it->size - segs_needed <= 256) {
          // Just make ram used, and return the existing block.
          it->avail = memory_avail_t::used;
          return {it->start};
        }
        auto b = make_block(it->start + segs_needed, it->size - segs_needed, memory_avail_t::free);
        // shrink current free block and assign the new one added after it
        it->size = segs_needed;
        it->avail = memory_avail_t::used;
        // Save start pos to return it later.
        const auto start = it->start;
        blocks_.emplace(it + 1, std::move(b));
        return {start};
      }
    }
  }
  // not enough memory.
  return std::nullopt;
}

void DosMemoryManager::free(uint16_t seg) {
  // TODO(rushfan): Implement better version of free that merges free blocks.
  for (auto& it = std::begin(blocks_); it != std::end(blocks_); ++it) {
    if (it->start == seg && it->avail == memory_avail_t::used) {
      it->avail = memory_avail_t::free;
      it->owner = 0;
      return;
    }
  }
}

Dos::Dos(door86::cpu::x86::CPU* cpu) : cpu_(cpu), mem_mgr(&cpu->memory) {
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
  VLOG(3) << fmt::format("[{:04x}:{:04x}] DOS Interrupt: 0x{:04x}; {:02X}", cpu_->core.sregs.cs,
                           cpu_->core.ip, cpu_->core.regs.x.ax,
                           static_cast<int>(cpu_->core.regs.h.ah));
  switch (cpu_->core.regs.h.ah) {
  // terminate app
  case 0x00: cpu_->halt(); break;
  // read char
  case 0x01: get_char(); break;
  // display char
  case 0x02: display_char(); break;
  // display string
  case 0x9: display_string(); break;
  // INT 21 - AH = 25h DOS - SET INTERRUPT VECTOR
  case 0x25: set_interrupt_vector(); break;
  // INT 21 - DOS 2+ - GET DOS VERSION
  case 0x30: getversion(); break;
  // Get Interrupt Vector
  case 0x35: get_interrupt_vector(); break;
  case 0x40: dos_write(); break;
  // terminate app.
  case 0x4c:
    VLOG(2) << "Terminate App";
    cpu_->halt();
    break;
  case 0x58: memory_strategy(); break;
  case 0x67: set_handle_count(); break;
  default: {
    // unhandled
    VLOG(2) << "Unhandled DOS Interrupt "
            << fmt::format("AH:{:02X}; AL:{:02X}", cpu_->core.regs.h.ah, cpu_->core.regs.h.al);
  } break;
  }
}

void Dos::getversion() {
  VLOG(2) << "Get DOS Version";
  cpu_->core.regs.x.ax = 0x0005;
  cpu_->core.regs.x.bx = 0x0000;
  cpu_->core.regs.x.cx = 0x0000;
}

void Dos::get_interrupt_vector() {
  const uint8_t v = cpu_->core.regs.h.al;
  uint16_t off = cpu_->memory.get<uint16_t>(0, v * 4);
  uint16_t seg = cpu_->memory.get<uint16_t>(0, (v * 4) + 2);
  VLOG(2) << fmt::format("Get Interrupt Vector for: {:02X} -> {:04X}{:04X}", v, seg, off);
  cpu_->memory.set<uint16_t>(cpu_->core.sregs.es, cpu_->core.regs.x.bx, off);
  cpu_->memory.set<uint16_t>(cpu_->core.sregs.es, cpu_->core.regs.x.bx + 2, seg);
}

void Dos::set_interrupt_vector() {
  auto v = cpu_->core.regs.h.ah;
  auto off = cpu_->memory.get<uint16_t>(cpu_->core.sregs.ds, cpu_->core.regs.x.dx);
  auto seg = cpu_->memory.get<uint16_t>(cpu_->core.sregs.ds, cpu_->core.regs.x.dx + 2);

  cpu_->memory.set<uint16_t>(0, v * 4, off);
  cpu_->memory.set<uint16_t>(0, (v * 4) + 2, seg);
}

void Dos::display_char() { fputc(cpu_->core.regs.h.dl, stdout); }

void Dos::display_string() {
  for (auto offset = cpu_->core.regs.x.dx;; ++offset) {
    const auto m = cpu_->memory.get<uint8_t>(cpu_->core.sregs.ds, offset);
    if (m == '$' || m == '\0') {
      // TODO(rushfan): We shouldn't stop at \0, but we will for now.
      break;
    }
    fputc(m, stdout);
  }
}

void Dos::get_char() { cpu_->core.regs.h.al = static_cast<uint8_t>(fgetc(stdin)); }

/*
  AH = 40h
  BX = file handle
  CX = number of bytes to write, a zero value truncates/extends
        the file to the current file position
  DS:DX = pointer to write buffer

  0 - Standard Input Device - can be redirected (STDIN)
  1 - Standard Output Device - can be redirected (STDOUT)
  2 - Standard Error Device - can be redirected (STDERR)
  3 - Standard Auxiliary Device (STDAUX)
  4 - Standard Printer Device (STDPRN) 
 */
void Dos::dos_write() {
  VLOG(1) << "dos_write: ";
  const auto h = cpu_->core.regs.x.bx;
  FILE* f = stdout;
  if (h < 5) {
    if (h == 2) {
      f = stderr;
    }
  } else {
    LOG(WARNING) << "Writing to files not yet supported";
    return;
  }
  auto* b = &cpu_->memory[(cpu_->core.sregs.ds * 0x10) + cpu_->core.regs.x.dx];
  fwrite(b, 1, cpu_->core.regs.x.cx, f);
}


void Dos::memory_strategy() { 
  switch (cpu_->core.regs.h.al) {
  case 0: //get
    cpu_->core.regs.x.bx = 2;
    break;
  case 1: // set
    // DO nothing.
    break;
  }
}

void Dos::set_handle_count() {
  // NOP - success
  cpu_->core.flags.cflag(false);
}

} // namespace door86::dos
