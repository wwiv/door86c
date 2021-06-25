#include "cpu/memory.h"

namespace door86::cpu{


Memory::Memory(int size) : size_(size) {
  mem_ = new uint8_t[size];
}

Memory::~Memory() {
  delete mem_;
  mem_ = nullptr;
}

}