#ifndef INCLUDED_DOS_PSP_H
#define INCLUDED_DOS_PSP_H

#include <cstdint>

namespace door86::dos {

#pragma pack(push, 1)
// https://en.wikipedia.org/wiki/Program_Segment_Prefix

struct psp_t {
  uint16_t int20_instruction;
  uint16_t ending_address;
  uint8_t reserved1;
  uint8_t call_to_dos_fn_dispatcher[5];
  uint32_t addr_termination_code;
  uint32_t addr_break_handler;
  uint32_t addr_crit_error_handler;
  uint16_t parent_psp_segment;
  uint8_t job_file_table[22];
  uint16_t environ_seg;
  uint32_t last_int21_sssp;
  uint16_t jft_size;
  uint16_t jft_pointer;
  uint32_t previous_psp_ptr;
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

  psp_t* psp;
};

} // namespace door86::dos

#endif