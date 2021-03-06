#include "cpu/memory.h"

#include <cstring>

namespace door86::cpu {

// returns the absolute memory location for a segmented address
static inline uint32_t abs_memory(const seg_address_t& loc) { 
  return (loc.seg * 0x10) + loc.off; 
}
// returns the absolute memory location for a segmented address
static inline uint32_t abs_memory(uint16_t seg, uint16_t off) {
  return (seg * 0x10) + off; 
}

Memory::Memory(int size) : size_(size) { 
  mem_ = new uint8_t[size];
  memset(mem_, 0, size);
}

Memory::~Memory() {
  delete mem_;
  mem_ = nullptr;
}

bool Memory::load_image(size_t start, size_t size, const uint8_t* image) {
  if (size_ - start < size) {
    // We can't load the image, too big to fit.
    return false;
  }
  memmove(mem_ + start, image, size);
  return true;
}

bool Memory::load_image(const seg_address_t& start, size_t size, const uint8_t* image) {
  return load_image(abs_memory(start), size, image);
}

// loads an image of size (size) into memory starting at absolute location start
bool Memory::load_string(size_t start, const std::string& s) {
  if (size_ - start < s.size()) {
    // We can't load the image, too big to fit.
    return false;
  }
  return load_image(start, s.size(), reinterpret_cast<const uint8_t*>(s.data()));
}

// clears (zeros) a block of memory
bool Memory::clear(size_t start, size_t size) {
  if (size_ - start < size) {
    // We can't load the image, too big to fit.
    return false;
  }
  memset(mem_ + start, 0, size);
  return true;
}

// returns value from an absolute memory location
uint16_t Memory::abs16(uint32_t loc) const {
  const auto* p = reinterpret_cast<uint16_t*>(mem_ + loc);
  return *p;
}

// sets a value from an absolute memory location
void Memory::abs16(uint32_t loc, uint16_t value) {
  auto* p = reinterpret_cast<uint16_t*>(mem_ + loc);
  *p = value;
}


} // namespace door86::cpu