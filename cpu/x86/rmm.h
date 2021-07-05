#ifndef INCLUDED_CPU_X86_RMM_H
#define INCLUDED_CPU_X86_RMM_H

#include "cpu/memory.h"
#include "cpu/x86/cpu_core.h"
#include <cstdint>
#include <type_traits>

// Start with instructons needed for hello world in asm, then expand
// to these, then on to others as needed.
// https://github.com/xem/minix86/blob/gh-pages/src/instructions.js

namespace door86::cpu::x86 {

enum class RmmType { EITHER, MEMORY, REGISTER };

/**
 * Represents the value owned by the ModR/M byte, it can point to either
 * a register or a memory location.  Updates to the value of this class
 * by operators on it will be reflected in either the register or
 * memory that this class represents.
 */
template <RmmType R, typename T> class Rmm {
public:

  // A type that is one larger than T, used to detect overflow
  using bigT = std::conditional_t<std::is_same_v<T, uint8_t>, uint16_t,
               std::conditional_t<std::is_same_v<T, uint16_t>, uint32_t, void>>;
  // FIXME
  // TODO(rushfan): Our CF and OF handling is completely broken.
  static constexpr bigT cf_mask = std::numeric_limits<bigT>::max() - std::numeric_limits<T>::max();
  static constexpr bigT of_mask =
      std::numeric_limits<bigT>::max() - std::numeric_limits<std::make_signed_t<T>>::max();
  static constexpr auto digits = std::numeric_limits<T>::digits;
  static constexpr auto msb_mask = (1 << (digits - 1));
  static constexpr auto remainder_mask = ~msb_mask;

  Rmm(cpu_core* core, T* reg) :core_(core),  mem_(nullptr), reg_(reg), seg_(0), off_(0) {}
  Rmm(cpu_core* core, Memory* mem, uint16_t seg, uint16_t off)
      : core_(core), mem_(mem), reg_(nullptr), seg_(seg), off_(off) {}

  inline T get() const {
    if constexpr (R == RmmType::REGISTER) {
      return *reg_;
    } 
    if constexpr (R == RmmType::MEMORY) {
      return mem_->get<T>(seg_, off_);
    } 
    if (mem_) {
      return mem_->get<T>(seg_, off_);
    }
    return *reg_;
  }

  inline void set(T v) {
    if constexpr (R == RmmType::REGISTER) {
      *reg_ = v;
    }
    if constexpr (R == RmmType::MEMORY) {
      mem_->set<T>(seg_, off_, v);
    }
    if (mem_) {
      mem_->set<T>(seg_, off_, v);
    } else {
      *reg_ = v;
    }
  }

  inline void update_flags_psz() {
    const auto cur = get();
    set_flags_psz(cur);
  }

  inline void set_flags_psz(const T& cur) {
    core_->flags.pflag((kern_popcount(cur) % 2) == 0);
    core_->flags.zflag(cur == 0);
    if constexpr (std::is_same_v<T, uint8_t>) {
      core_->flags.sflag(cur & 0x80);
    } else if constexpr (std::is_same_v<T, uint16_t>) {
      core_->flags.sflag(cur & 0x8000);
    } else {
      static_assert(false, "needs uint8_t or uint16_t");
    }
  }

  inline T set_flags_from_bigt(bigT& cur) {
    core_->flags.oflag(cur & of_mask);
    core_->flags.cflag(cur & cf_mask);

    const auto adj_cur = static_cast<T>(cur);
    set_flags_psz(adj_cur);
    return adj_cur;
  }

  inline Rmm& operator+=(const T& other) {
    bigT cur = get();
    cur += other;
    set(set_flags_from_bigt(cur));
    return *this;
  }

  inline Rmm& operator+=(const Rmm<RmmType::MEMORY, T>& other) { return operator+=(other.get()); }
  inline Rmm& operator+=(const Rmm<RmmType::REGISTER, T>& other) { return operator+=(other.get()); }
  inline Rmm& operator+=(const Rmm<RmmType::EITHER, T>& other) { return operator+=(other.get()); }

  inline Rmm& operator-=(const T& other) {
    bigT cur = get();
    cur -= other;
    set(set_flags_from_bigt(cur));
    return *this;
  }

  inline Rmm& operator-=(const Rmm<RmmType::MEMORY, T>& other) { return operator-=(other.get()); }
  inline Rmm& operator-=(const Rmm<RmmType::REGISTER, T>& other) { return operator-=(other.get()); }
  inline Rmm& operator-=(const Rmm<RmmType::EITHER, T>& other) { return operator-=(other.get()); }

  void cmp(const T& other) {
    // Compares the first source operand with the second source operand and sets
    // the status flags in the EFLAGS register according to the results. 
    // 
    // The comparison is performed by subtracting the second operand from the
    // firstoperand and then setting the status flags in the same manner as
    // the SUB instruction.  When an immediate value is used as an operand, 
    // it is sign-extended to the length of the first operand.
    T saved = get();
    bigT cur = saved;
    cur -= other;
    set_flags_from_bigt(cur);
  }

  inline void cmp(const Rmm<RmmType::MEMORY, T>& other) { return cmp(other.get()); }
  inline void cmp(const Rmm<RmmType::REGISTER, T>& other) { return cmp(other.get()); }
  inline void cmp(const Rmm<RmmType::EITHER, T>& other) { return cmp(other.get()); }


  inline Rmm& operator|=(const T& other) {
    T cur = get();
    cur |= other;
    set_flags_psz(cur);
    set(cur);
    return *this;
  }
  inline Rmm& operator|=(const Rmm<RmmType::MEMORY, T>& other) { return operator|=(other.get()); }
  inline Rmm& operator|=(const Rmm<RmmType::REGISTER, T>& other) { return operator|=(other.get()); }
  inline Rmm& operator|=(const Rmm<RmmType::EITHER, T>& other) { return operator|=(other.get()); }

  inline Rmm& operator^=(const T& other) {
    T cur = get();
    cur ^= other;
    set_flags_psz(cur);
    set(cur);
    return *this;
  }
  inline Rmm& operator^=(const Rmm<RmmType::MEMORY, T>& other) { return operator^=(other.get()); }
  inline Rmm& operator^=(const Rmm<RmmType::REGISTER, T>& other) { return operator^=(other.get()); }
  inline Rmm& operator^=(const Rmm<RmmType::EITHER, T>& other) { return operator^=(other.get()); }

  // AND

  inline Rmm& operator&=(const T& other) {
    T cur = get();
    cur &= other;
    set_flags_psz(cur);
    set(cur);
    return *this;
  }
  inline Rmm& operator&=(const Rmm<RmmType::MEMORY, T>& other) { return operator&=(other.get()); }
  inline Rmm& operator&=(const Rmm<RmmType::REGISTER, T>& other) { return operator&=(other.get()); }
  inline Rmm& operator&=(const Rmm<RmmType::EITHER, T>& other) { return operator&=(other.get()); }

  inline Rmm& neg() {
    bigT cur;
    if constexpr (std::is_same_v<T, uint8_t>) {
      cur = 0x100 - get();
    } else 
    if constexpr (std::is_same_v<T, uint16_t>) {
      cur = 0x10000 - get();
    } else {
      static_assert(false, "needs uint8_t or uint16_t");
    }
    set(set_flags_from_bigt(cur));
    return *this;
  }

  inline Rmm& shl(int num) { 
    auto v = get();
    core_->flags.cflag(v & msb_mask);
    v = (v << num);
    set(v);
    set_flags_psz(v);
    return *this;
  }

  inline Rmm& rol(int num) {
    auto v = get();
    core_->flags.cflag(v & msb_mask);
    const auto lsb = (v & msb_mask) ? 1 : 0;
    v = (v << num) | lsb;
    set(v);
    set_flags_psz(v);
    return *this;
  }

  // rotate left with carry
  inline Rmm& rcl(int num) {
    auto v = get();
    const auto lsb = core_->flags.cflag() ? 1 : 0;
    core_->flags.cflag(v & msb_mask);
    v = (v << num) | lsb;
    set(v);
    set_flags_psz(v);
    return *this;
  }

  inline Rmm& shr(int num) {
    auto v = get();
    core_->flags.cflag(v & 1);
    v = (v >> num);
    set(v);
    set_flags_psz(v);
    return *this;
  }

  // rotate right with carry
  inline Rmm& rcr(int num) {
    auto v = get();
    const auto msb = core_->flags.cflag() ? msb_mask : 0;
    core_->flags.cflag(v & 1);
    v = (v >> num) | msb;
    set(v);
    set_flags_psz(v);
    return *this;
  }

  // rotate right
  inline Rmm& ror(int num) {
    auto v = get();
    core_->flags.cflag(v & 1);
    const auto msb = (v & 1) ? msb_mask : 0;
    v = (v >> num) | msb;
    set(v);
    set_flags_psz(v);
    return *this;
  }

  inline Rmm& sar(int num) {
    auto v = get();
    core_->flags.cflag(v & 1);
    // if we have the sign bit
    const bool neg = (v & msb_mask) != 0;
    // clear the sign bit
    v &= ~msb_mask;
    // shift to the right
    v = v >> num;
    if (neg) {
      // restore the sign bit
      v |= msb_mask;
    }
    set(v);
    set_flags_psz(v);
    return *this;
  }

private:
  cpu_core* core_;
  Memory* mem_;
  T* reg_;
  const uint16_t seg_;
  const uint16_t off_;
};

/**
 * Swap Rmms
 */
template <class R1, class R2> void swap(R1 left, R2 right) {
  const auto tempr = left.get();
  left.set(right.get());
  right.set(tempr);
}


// calaculates the effective address encoded in the instruction
uint16_t effective_address(const instruction_t& inst, const cpu_core& core);

// Creates an Rmm for a 16bit register specified in the instrution, heeding any segment register overrides
Rmm<RmmType::REGISTER, uint16_t> r16(const instruction_t& inst, cpu_core& core);

// Creates an Rmm for a 8bit register specified in the instrution, heeding any segment register
// overrides
Rmm<RmmType::REGISTER, uint8_t> r8(const instruction_t& inst, cpu_core& core);

// Creates an Rmm for a 8bit register or memory location specified in the instrution, heeding any
// segment register overrides
Rmm<RmmType::EITHER, uint8_t> rmm8(const instruction_t& inst, cpu_core& core, Memory& mem);

// Creates an Rmm for a 16bit register or memory location specified in the instrution, heeding any
// segment register overrides
Rmm<RmmType::EITHER, uint16_t> rmm16(const instruction_t& inst, cpu_core& core, Memory& mem);


} // namespace door86::cpu::x86

#endif
