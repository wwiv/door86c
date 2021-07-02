#ifndef INCLUDED_CPU_X86_RMM_H
#define INCLUDED_CPU_X86_RMM_H

#include "cpu/memory.h"
#include "cpu/x86/core.h"
#include <cstdint>
#include <type_traits>

// Start with instructons needed for hello world in asm, then expand
// to these, then on to others as needed.
// https://github.com/xem/minix86/blob/gh-pages/src/instructions.js

namespace door86::cpu::x86 {

/**
 * Represents the value owned by the ModR/M byte, it can point to either
 * a register or a memory location.  Updates to the value of this class
 * by operators on it will be reflected in either the register or
 * memory that this class represents.
 */
template <typename T> class Rmm {
public:

  // A type that is one larger than T, used to detect overflow
  using bigT = std::conditional_t<std::is_same_v<T, uint8_t>, uint16_t,
               std::conditional_t<std::is_same_v<T, uint16_t>, uint32_t, void>>;
  static constexpr bigT cf_mask = std::numeric_limits<bigT>::max() - std::numeric_limits<T>::max();
  static constexpr bigT of_mask =
      std::numeric_limits<bigT>::max() - std::numeric_limits<std::make_signed_t<T>>::max();

  Rmm(cpu_core* core, T* reg) :core_(core),  mem_(nullptr), reg_(reg), seg_(0), off_(0) {}
  Rmm(cpu_core* core, Memory* mem, uint16_t seg, uint16_t off)
      : core_(core), mem_(mem), reg_(nullptr), seg_(seg), off_(off) {}

  inline T get() const {
    if (mem_) {
      return mem_->get<T>(seg_, off_);
    }
    return *reg_;
  }

  inline void set(T v) {
    if (mem_) {
      mem_->set<T>(seg_, off_, v);
    } else {
      *reg_ = v;
    }
  }

  inline T set_flags(bigT& cur) {
    if (cur & of_mask) {
      core_->flags.set(OF);
    }
    if (cur & cf_mask) {
      core_->flags.set(CF);
    }

    T adj_cur = static_cast<T>(cur);
    if ((kern_popcount(adj_cur) % 2) == 0) {
      core_->flags.set(PF);
    } else {
      core_->flags.clear(PF);
    }
    if (adj_cur == 0) {
      core_->flags.set(ZF);
    } else {
      core_->flags.clear(ZF);
    }
    return adj_cur;
  }

  inline Rmm& operator+=(const T& other) {
    bigT cur = get();
    cur += other;
    set(set_flags(cur));
    return *this;
  }

  inline Rmm& operator-=(const T& other) {
    bigT cur = get();
    cur -= other;
    set(set_flags(cur));
    return *this;
  }

  inline Rmm& operator|=(const T& other) {
    bigT cur = get();
    cur |= other;
    set(set_flags(cur));
    return *this;
  }

  inline Rmm& operator^=(const T& other) {
    bigT cur = get();
    cur ^= other;
    set(set_flags(cur));
    return *this;
  }

private:
  cpu_core* core_;
  Memory* mem_;
  T* reg_;
  const uint16_t seg_;
  const uint16_t off_;
};


uint16_t r16(const instruction_t& inst, cpu_core& core);
uint16_t effective_address(const instruction_t& inst, const cpu_core& core);

Rmm<uint8_t> rmm8(const instruction_t& inst, cpu_core& core, Memory& mem);
Rmm<uint16_t> rmm16(const instruction_t& inst, cpu_core& core, Memory& mem);

} // namespace door86::cpu::x86

#endif
