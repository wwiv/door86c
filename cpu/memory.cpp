#include "cpu/memory.h"

#include <cstring>

namespace door86::cpu {

namespace {
// returns the absolute memory location for a segmented address
static inline uint32_t abs_memory(const seg_address_t& loc) { return (loc.seg * 0x10) + loc.off; }
// returns the absolute memory location for a segmented address
static inline uint32_t abs_memory(uint16_t seg, uint16_t off) { return (seg * 0x10) + off; }
}

Memory::Memory(int size) : size_(size) { mem_ = new uint8_t[size]; }

Memory::~Memory() {
  delete mem_;
  mem_ = nullptr;
}

bool Memory::load_image(uint32_t start, uint32_t size, uint8_t* image) {
  if (size_ - start < size) {
    // We can't load the image, too big to fit.
    return false;
  }
  memmove(mem_ + start, image, size);
  return true;
}

bool Memory::load_image(const seg_address_t& start, uint32_t size, uint8_t* image) {
  return load_image(abs_memory(start), size, image);
}

uint8_t* Memory::absbyte(uint32_t loc) { return mem_ + loc; }
uint8_t& Memory::absbyteref(uint32_t loc) { return mem_[loc]; }
uint16_t* Memory::absword(uint32_t loc) { return reinterpret_cast<uint16_t*>(mem_ + loc); }
uint16_t& Memory::abswordref(uint32_t loc) { return reinterpret_cast<uint16_t&>(mem_[loc]); }

uint8_t* Memory::byte(const seg_address_t& loc) { return absbyte(abs_memory(loc)); }
uint8_t& Memory::byteref(const seg_address_t& loc) { return absbyteref(abs_memory(loc)); }

uint8_t* Memory::byte(uint16_t seg, uint16_t off) { return absbyte(abs_memory(seg, off)); }
uint8_t& Memory::byteref(uint16_t seg, uint16_t off) { return absbyteref(abs_memory(seg, off)); }

uint16_t* Memory::word(const seg_address_t& loc) { return absword(abs_memory(loc)); } 
uint16_t& Memory::wordref(const seg_address_t& loc) { return abswordref(abs_memory(loc)); } 

uint16_t* Memory::word(uint16_t seg, uint16_t off) { return absword(abs_memory(seg, off)); }
uint16_t& Memory::wordref(uint16_t seg, uint16_t off) { return abswordref(abs_memory(seg, off)); }

} // namespace door86::cpu