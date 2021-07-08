#ifndef INCLUDED_CPU_MEMORY_H
#define INCLUDED_CPU_MEMORY_H

#include "core/log.h"
#include "cpu/memory_bits.h"
#include <cstdint>
#include <string>

namespace door86::cpu {


class Memory final {
private:
  // returns the absolute memory location for a segmented address
  static inline uint32_t abs_memory(const seg_address_t& loc) { return (loc.seg * 0x10) + loc.off; }
  // returns the absolute memory location for a segmented address
  static inline uint32_t abs_memory(uint16_t seg, uint16_t off) { return (seg * 0x10) + off; }

public:
  Memory(int size);
  ~Memory();

  const uint8_t& operator[](int loc) const { return mem_[loc]; }
  uint8_t& operator[](int loc) { return mem_[loc]; }

  // returns value from an absolute memory location
  uint8_t abs8(uint32_t loc) const { return mem_[loc]; }
  // sets a value from an absolute memory location
  void abs8(uint32_t loc, uint8_t value) { mem_[loc] = value; }

  // returns value from an absolute memory location
  uint16_t abs16(uint32_t loc) const;
  // sets a value from an absolute memory location
  void abs16(uint32_t loc, uint16_t value);

  template <typename T> void set(uint16_t seg, uint16_t off, T value) {
    if constexpr (std::is_same_v<T, uint8_t>) {
      abs8(Memory::abs_memory(seg, off), value);
    } else if constexpr (std::is_same_v<T, uint16_t>) {
      abs16(Memory::abs_memory(seg, off), value);
    } else {
      static_assert(false, "needs uint8_t or uint16_t");
    }
  }

  template <typename T> T get(uint16_t seg, uint16_t off) const {
    if constexpr (std::is_same_v<T, uint8_t>) {
      return abs8(Memory::abs_memory(seg, off));
    } else if constexpr (std::is_same_v<T, uint16_t>) {
      return abs16(Memory::abs_memory(seg, off));
    } else {
      static_assert(false, "needs uint8_t or uint16_t");
    }
  }

  // loads an image of size (size) into memory starting at absolute location start
  bool load_image(size_t start, size_t size, const uint8_t* image);
  // loads an image of size (size) into memory starting at segmented location start
  bool load_image(const seg_address_t& start, size_t size, const uint8_t* image);

  // Templatized memory access

  // Gets a pointer to a memory location.
  template <class T> T* ptr(uint16_t seg, uint16_t off) const {
    const auto loc = abs_memory(seg, off);
    CHECK_LE(loc + sizeof(T), size_);
    return reinterpret_cast<T*>(mem_ + loc);
  }

  // Gets a pointer to a memory location and zeros out the block.
  template <class T> T* ptr_zero(uint16_t seg, uint16_t off) const {
    const auto loc = abs_memory(seg, off);
    CHECK_LE(loc + sizeof(T), size_);
    auto* p reinterpret_cast<T*>(mem_ + loc);
    memset(p, '\0', sizeof(T));
    return p;
  }

  // Helpers for testing

  // loads an image of size (size) into memory starting at absolute location start
  bool load_string(size_t start, const std::string& s);
  // clears (zeros) a block of memory
  bool clear(size_t start, size_t size);

private:
  const int size_;
  bool debug_{false};
  uint8_t* mem_;
};

} // namespace door86::cpu

#endif // INCLUDED_CPU_MEMORY_H