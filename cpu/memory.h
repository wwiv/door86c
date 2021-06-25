#ifndef INCLUDED_CPU_MEMORY_H
#define INCLUDED_CPU_MEMORY_H

#include <cstdint>

namespace door86::cpu{

class Memory {
public:
  Memory(int size);
  ~Memory();

  const uint8_t& operator[](int loc) const { return mem_[loc]; }
  uint8_t& operator[](int loc) { return mem_[loc]; }

private:
  const int size_;
  bool debug_{ false };
  uint8_t* mem_;
};

}

#endif  // INCLUDED_CPU_MEMORY_H