#ifndef INCLUDED_DOS_PSP_H
#define INCLUDED_DOS_PSP_H

#include "cpu/memory_bits.h"
#include <cstdint>
#include <string>

namespace door86::dos {

#pragma pack(push, 1)
// https://en.wikipedia.org/wiki/Program_Segment_Prefix
// Also http://www.delorie.com/djgpp/doc/rbinter/it/78/13.html

struct psp_t {
  uint8_t int20_instruction[2];
  uint16_t ending_address;
  uint8_t reserved1;
  uint8_t call_to_dos_fn_dispatcher[5];
  door86::cpu::seg_address_t addr_termination_code;
  door86::cpu::seg_address_t addr_break_handler;
  door86::cpu::seg_address_t addr_crit_error_handler;
  uint16_t parent_psp_segment;
  uint8_t job_file_table[20];
  uint16_t environ_seg;
  door86::cpu::seg_address_t last_int21_sssp;
  uint16_t jft_size;
  door86::cpu::seg_address_t jft_pointer;
  door86::cpu::seg_address_t previous_psp_ptr;
  uint32_t reserved2;
  uint16_t dos_version_to_return;
  uint8_t reserved3[14];
  uint8_t int21_retf_instructions[3];
  uint8_t reserved4[9];
  uint8_t fcb1[16];
  uint8_t fcb2[20];
  uint8_t cmdlen_length;
  char cmdline[127];
};
#pragma pack(pop)

class PSP final {
public:
  PSP(void* memory) { psp = reinterpret_cast<psp_t*>(memory); }
  ~PSP() = default;

  /** Initializes a default PSP for a new process on top of an existing memory area */
  bool initialize();
  /** Sets the args of the commandline */
  void set_commandline(std::string args);

  psp_t* psp;
};

} // namespace door86::dos

#endif