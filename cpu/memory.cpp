#include "cpu/memory.h"

#include <cstring>

namespace door86::cpu{


Memory::Memory(int size) : size_(size) {
  mem_ = new uint8_t[size];
}

Memory::~Memory() {
  delete mem_;
  mem_ = nullptr;
}

bool Memory::load_image(int start, int size, uint8_t* image) {
  if (size_ - start < size) {
    // We can't load the image, too big to fit.
    return false;
  }
  memmove(mem_ + start, image, size);
  return true;
}


}