#ifndef INCLUDED_DOS_DOS_H
#define INCLUDED_DOS_DOS_H

#include "cpu/memory.h"
#include "cpu/x86/cpu.h"
#include "dos/psp.h"

#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace door86::dos {

class DosMemoryManager {
public:
  DosMemoryManager() : DosMemoryManager(0x0800, 0x9FC0) {}
  DosMemoryManager(uint16_t start_seg, uint16_t end_seg)
      : start_seg_(start_seg), end_seg_(end_seg), top_seg_(start_seg_) {}
  ~DosMemoryManager() = default;

  // allocate a block of memory of size bytes, returns the starting segment;
  std::optional<uint16_t> allocate(size_t size);
  void free(uint16_t seg);

private:
  // See http://staff.ustc.edu.cn/~xyfeng/research/cos/resources/machine/mem.htm
  // First free block after boot sector code (rounded to 0x100)
  uint16_t start_seg_{0x0800};
  // 9FC00- 9FFFF	extended BIOS data area (EBDA)
  // 0xA000:0x0000	64 Kb	Graphics Video Memory
  // but even this gives us >600K available.
  // TODO(rushfan) Could we possibly move all the way up to 0xB800 or 0xB000)??
  uint16_t end_seg_{0x9FC0};
  uint16_t top_seg_{start_seg_};
};

class Dos {
public:
  Dos(door86::cpu::x86::CPU* cpu);
  ~Dos() = default;
  // Initializes the PSP and anything else needed before starting
  // DS contains the PSP segment.
  bool initialize_process(const std::filesystem::path& filename);

  void int20(int, door86::cpu::x86::CPU&);
  void int21(int, door86::cpu::x86::CPU&);

  std::unique_ptr<PSP> psp_;
  DosMemoryManager mem_mgr;
  door86::cpu::x86::CPU* cpu_{nullptr};
};

/*
    Address	      Size	      Name
    ============= ----------- =============================
    0x0000:0x0000	1024 bytes	Interrupt Vector Table
    0x0040:0x0000	256 bytes	  BIOS Data Area
    0x0050:0x0000	?	          Free memory
    0x07C0:0x0000	512 bytes	  Boot sector code
    0x07E0:0x0000	?	          Free memory
    0xA000:0x0000	64 Kb	      Graphics Video Memory
    0xB000:0x0000	32 Kb	      Monochrome Text Video Memory
    0xB800:0x0000	32 Kb	Color Text Video Memory
    0xC000:0x0000	256 Kb1	    ROM Code Memory
    0xFFFF:0x0000	16 bytes	  More BIOS data 
*/

}

#endif // INCLUDED_DOS_DOS_H