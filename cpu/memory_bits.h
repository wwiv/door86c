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

}

#endif // INCLUDED_CPU_MEMORY_BITS_H