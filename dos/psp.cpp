#include "dos/psp.h"

namespace door86::dos {

bool PSP::initialize() {
  psp->int20_instruction[0] = 0xCD;
  psp->int20_instruction[1] = 0x2a;
  psp->ending_address = 0xFF;

  psp->dos_version_to_return = 0x05;
  // todo: add int21_retf_instructions and cmdline

  return false;
};

}


