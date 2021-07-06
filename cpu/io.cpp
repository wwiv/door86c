#include "cpu/io.h"

#include "core/log.h"
#include <cstring>

namespace door86::cpu {

uint8_t IO::inb(uint16_t port) {
  LOG(INFO) << "Called inb(" << port << ")";
  return 0;
}

uint16_t IO::inw(uint16_t port) {
  LOG(INFO) << "Called inw(" << port << ")";
  return 0;
}

void IO::outb(uint16_t port, uint8_t value) { LOG(INFO) << "Called outb(" << port << ", " << value << ")"; }

void IO::outw(uint16_t port, uint16_t value) { LOG(INFO) << "Called outw(" << port << ", " << value << ")"; }

} // namespace door86::cpu