#include "bios/bios.h"

namespace door86::bios {

Bios::Bios(door86::cpu::x86::CPU* cpu) : cpu_(cpu) {
  cpu_->int_handlers().try_emplace(
      0x10, std::bind(&Bios::int10, this, std::placeholders::_1, std::placeholders::_2));
}

void Bios::int10(int, door86::cpu::x86::CPU&) {
  auto& r = cpu_->core.regs;
  switch (r.h.ah) {
  // INT 10, 13 - Write String(BIOS versions from 1 / 10 / 86)
  case 0x13: {
    if (r.h.al > 1) {
      LOG(WARNING) << "Colors not yet supported";
    }
    for (int i = 0; i < r.x.cx; i++) {
      fputc(cpu_->memory.get<uint8_t>(cpu_->core.sregs.es, r.x.bp + i), stdout);
    }
    fflush(stdout);
  } break;
  // INT 10,E - Write Text in Teletype Mode
  case 0x0E: {
    fputc(r.h.al, stdout);
    fflush(stdout);
  } break;
  } // switch
}

} // namespace door86::bios
