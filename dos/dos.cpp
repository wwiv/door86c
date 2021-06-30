#include "dos/dos.h"

#include "core/file.h"
#include "core/log.h"
#include "core/scope_exit.h"
#include "fmt/format.h"
#include <cstdio>
#include <optional>
#include <string>

// MSVC only has __PRETTY_FUNCTION__ in intellisense,
// TODO(rushfan): Find a better home for this macro.
#if !defined(__PRETTY_FUNCTION__)
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

namespace door86::dos {

using namespace wwiv::core;

// allocate a block of memory of size bytes, returns the starting segment;
std::optional<uint16_t> DosMemoryManager::allocate(size_t size) { 
  size_t segs_needed = 1 + (size / 16);
  if (end_seg_ - start_seg_ < segs_needed) {
    // not enough memory.
    return std::nullopt;
  }
  auto seg = top_seg_;
  // to start with we'll load from the bottom
  top_seg_ += static_cast<uint16_t>(segs_needed);
  return {seg};
}

void DosMemoryManager::free(uint16_t seg) {
  //TODO(rushfan): Implement free
}

}

