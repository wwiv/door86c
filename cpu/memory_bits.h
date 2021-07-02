#ifndef INCLUDED_CPU_MEMORY_BITS_H
#define INCLUDED_CPU_MEMORY_BITS_H

#include <cstdint>

#ifdef __GNUC__
// Otherwise we're likely on MSVC which isn't (on arm or x86)
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__ 
#define DOOR86_BIG_ENDIAN
#include <byteswap.h>
#endif
#endif

namespace door86::cpu {

/** Enum representing the segments */
enum class segment_t : int8_t { ES = 0, CS, SS, DS, FS, GS };

/** Structure representing a segmentted memory address */
#pragma pack(push, 1)
struct seg_address_t {
  uint16_t seg;
  uint16_t off;
};
#pragma pack(pop)

// m.b. __builtin_bswap16 is on gcc.
static_assert(sizeof(seg_address_t) == sizeof(uint32_t), "seg_address_t must be 4 bytes");

template <uint8_t idx, typename T> constexpr T bits() {
  return static_cast<T>(1) << idx;
}

template <uint8_t idx, typename T> inline void setBit(T& value) { value |= bits<idx, T>(); }
template <uint8_t idx, typename T> inline void clearBit(T& value) { value &= ~(bits<idx, T>()); }

// https://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetKernighan
static inline int kern_popcount(int n) {
  int c;
  for (c = 0; n; ++c) {
    n &= (n - 1);
  }
  return c;
}

}

#endif // INCLUDED_CPU_MEMORY_BITS_H