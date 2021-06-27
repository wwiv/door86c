#ifndef INCLUDED_CPU_MEMORY_H
#define INCLUDED_CPU_MEMORY_H

#include <cstdint>

namespace door86::cpu {

/** Structure representing a segmentted memory address */
struct seg_address_t {
  uint16_t seg;
  uint16_t off;
};

class Memory {
public:
  Memory(int size);
  ~Memory();

  const uint8_t& operator[](int loc) const { return mem_[loc]; }
  uint8_t& operator[](int loc) { return mem_[loc]; }

  // returns value from an absolute memory location
  uint8_t* absbyte(uint32_t loc);
  // returns value from an absolute memory location
  uint8_t& absbyteref(uint32_t loc);
  // returns value from an absolute memory location
  uint16_t* absword(uint32_t loc);
  // returns value from an absolute memory location
  uint16_t& abswordref(uint32_t loc);

  // returns a pointer to a segmented memory location
  uint8_t* byte(const seg_address_t& loc);
  // returns a reference to a segmented memory location
  uint8_t& byteref(const seg_address_t& loc);

  // returns a pointer to a segmented memory location
  uint8_t* byte(uint16_t seg, uint16_t off);
  // returns a reference to a segmented memory location
  uint8_t& byteref(uint16_t seg, uint16_t off);

  // returns a pointer to a segmented memory location
  uint16_t* word(const seg_address_t& loc);
  // returns a reference to a segmented memory location
  uint16_t& wordref(const seg_address_t& loc);

  // returns a pointer to a segmented memory location
  uint16_t* word(uint16_t seg, uint16_t off);
  // returns a reference to a segmented memory location
  uint16_t& wordref(uint16_t seg, uint16_t off);

  // loads an image of size (size) into memory starting at absolute location start
  bool load_image(uint32_t start, uint32_t size, uint8_t* image);
  // loads an image of size (size) into memory starting at segmented location start
  bool load_image(const seg_address_t& start, uint32_t size, uint8_t* image);

private:
  const int size_;
  bool debug_{false};
  uint8_t* mem_;
};

} // namespace door86::cpu

#endif // INCLUDED_CPU_MEMORY_H