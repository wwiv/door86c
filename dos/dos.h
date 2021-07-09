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

enum class memory_avail_t { free, used, system };
struct memory_block {
  // Available?
  memory_avail_t avail{memory_avail_t ::free};
  // segment start
  uint16_t start;
  // number of 16-byte paragraphs
  uint16_t size;
  // owner segment (should be start +1)
  uint16_t owner;
  std::string prog_name;
};

bool operator<(const memory_block& lhs, const memory_block& rhs);

bool operator==(const memory_block& lhs, const memory_block& rhs);


class DosMemoryManager {
public:
  enum class fit_strategy_t { first = 0, best = 1, last = 2 };

  explicit DosMemoryManager(door86::cpu::Memory* mem) : DosMemoryManager(mem, 0x0800, 0x9FC0) {}
  DosMemoryManager(door86::cpu::Memory* mem, uint16_t start_seg, uint16_t end_seg);
  ~DosMemoryManager() = default;

  // allocate a block of memory of size paragraphs, returns the starting segment for the MCB and also optional MCB;
  std::optional<uint16_t> allocate(uint16_t segs);

  // Finds a memory block located at seg.
  // seg will be the address of the MCB.
  bool free(uint16_t seg);

  // Finds a memory block located at seg.
  // seg will be the address of the MCB.
  std::optional<memory_block*> find(uint16_t seg);

  // Strategy
  fit_strategy_t strategy() const noexcept { return fit_; }
  void strategy(fit_strategy_t fit) { fit_ = fit; }

  // Visible for testing
  std::vector<memory_block>& blocks() { return blocks_; }

private:
  void write_mcb(const memory_block& b);

  door86::cpu::Memory* mem_;
  // See http://staff.ustc.edu.cn/~xyfeng/research/cos/resources/machine/mem.htm
  // First free block after boot sector code (rounded to 0x100)
  uint16_t start_seg_{0x0800};
  // 9FC00- 9FFFF	extended BIOS data area (EBDA)
  // 0xA000:0x0000	64 Kb	Graphics Video Memory
  // but even this gives us >600K available.
  // TODO(rushfan) Could we possibly move all the way up to 0xB800 or 0xB000)??
  uint16_t end_seg_{0x9FC0};
  fit_strategy_t fit_{fit_strategy_t::first};
  std::vector<memory_block> blocks_;
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
  door86::cpu::x86::CPU* cpu_;
  DosMemoryManager mem_mgr;

private:
  void getversion();
  void get_interrupt_vector();
  void set_interrupt_vector();

  // IO

  void display_char();
  void display_string();
  void get_char();
  void dos_write();
  void set_handle_count();

  // Memory
  void memory_strategy();
  void allocate();
  void free();
  // reallocate a memory block;
  void realloc();
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