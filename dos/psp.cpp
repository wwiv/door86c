#include "dos/psp.h"

namespace door86::dos {

bool PSP::initialize() {
  memset(psp, 0, sizeof(psp_t));
  psp->int20_instruction[0] = 0xCD;
  psp->int20_instruction[1] = 0x2a;
  // Is this right?? Who knows, but this is what I see in dosbox.
  psp->ending_address = 0x9FFF;

  psp->dos_version_to_return = 0x05;
  // todo: add int21_retf_instructions and cmdline
  psp->parent_psp_segment = 0xFFFE;
  return true;
};

void PSP::set_commandline(std::string args) {
  if (args.empty() || args.back() != 0x0d) {
    args.push_back(0x0d);
  }
  if (args.size() > 126) {
    args = args.substr(args.size() - 126);
  }
  psp->cmdlen_length = static_cast<uint8_t>(args.size() & 0xff);
  strcpy(psp->cmdline, args.c_str());
}

}


