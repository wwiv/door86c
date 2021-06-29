#ifndef INCLUDED_CPU_MEMORY_BITS_H
#define INCLUDED_CPU_MEMORY_BITS_H

#ifdef __GNUC__
// Otherwise we're likely on MSVC which isn't (on arm or x86)
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__ 
#define DOOR86_BIG_ENDIAN
#include <byteswap.h>
#endif
#endif

namespace door86::cpu {

// representes a 16 bit little endian value
class uint16_le_t final {
public:
  uint16_le_t& operator=(uint16_t o) {
#ifdef DOOR86_BIG_ENDIAN
    v = __builtin_bswap16(o);
#else
    v = o;
#endif
    return *this;
  }

// representes a 16 bit little endian value
  class uint32_le_t final {
  public:
    uint32_le_t& operator=(uint32_t o) {
#ifdef DOOR86_BIG_ENDIAN
      v = __builtin_bswap32(o);
#else
      v = o;
#endif
      return *this;
    }

  private:
  uint32_t v;
};

}

#endif // INCLUDED_CPU_MEMORY_BITS_H