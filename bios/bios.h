#ifndef INCLUDED_BIOS_BIOS_H
#define INCLUDED_BIOS_BIOS_H

#include "cpu/memory.h"
#include "cpu/x86/cpu.h"
#include "dos/psp.h"

#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace door86::bios {


class Bios {
public:
  Bios(door86::cpu::x86::CPU* cpu);
  ~Bios() = default;

  // INT 10 - Video BIOS Services
  void int10(int, door86::cpu::x86::CPU&);

  door86::cpu::x86::CPU* cpu_;

private:
  void display_char();
  void display_string();
  void get_char();
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