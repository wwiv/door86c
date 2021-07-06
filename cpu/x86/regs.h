#ifndef INCLUDED_CPU_X86_REGS_H
#define INCLUDED_CPU_X86_REGS_H

#include "core/log.h"
#include "cpu/memory.h"
#include "cpu/x86/decoder.h"
#include <cstdint>

// Start with instructons needed for hello world in asm, then expand
// to these, then on to others as needed.
// https://github.com/xem/minix86/blob/gh-pages/src/instructions.js

namespace door86::cpu::x86 {

struct regs16 {
  uint16_t ax;
  uint16_t cx;
  uint16_t dx;
  uint16_t bx;
  uint16_t sp;
  uint16_t bp;
  uint16_t si;
  uint16_t di;

  uint16_t get(int n) {
    switch (n) {
    case 0: return ax;
    case 1: return cx;
    case 2: return dx;
    case 3: return bx;
    case 4: return sp;
    case 5: return bp;
    case 6: return si;
    case 7: return di;
    default: LOG(FATAL) << "Invalid reg number encountered: " << n;
    }
    return 0;
  }

  uint16_t* regptr(int n) {
    switch (n) {
    case 0: return &ax;
    case 1: return &cx;
    case 2: return &dx;
    case 3: return &bx;
    case 4: return &sp;
    case 5: return &bp;
    case 6: return &si;
    case 7: return &di;
    default: LOG(FATAL) << "Invalid reg number encountered: " << n;
    }
    return nullptr;
  }

  void set(int n, uint16_t val) {
    switch (n) {
    case 0: ax = val; break;
    case 1: cx = val; break;
    case 2: dx = val; break;
    case 3: bx = val; break;
    case 4: sp = val; break;
    case 5: bp = val; break;
    case 6: si = val; break;
    case 7: di = val; break;
    default: LOG(FATAL) << "Invalid reg number encountered: " << n;
    }
  }
};

struct regs8 {
  uint8_t al, ah;
  uint8_t cl, ch;
  uint8_t dl, dh;
  uint8_t bl, bh;

  uint8_t get(int n) {
    switch (n) {
    case 0: return al;
    case 1: return cl;
    case 2: return dl;
    case 3: return bl;
    case 4: return ah;
    case 5: return ch;
    case 6: return dh;
    case 7: return bh;
    default: LOG(FATAL) << "Invalid reg number encountered: " << n;
    }
    return al;
  }

  void set(int n, uint8_t val) {
    switch (n) {
    case 0: al = val; break;
    case 1: cl = val; break;
    case 2: dl = val; break;
    case 3: bl = val; break;
    case 4: ah = val; break;
    case 5: ch = val; break;
    case 6: dh = val; break;
    case 7: bh = val; break;
    default: LOG(FATAL) << "Invalid reg number encountered: " << n;
    }
  }

  uint8_t* regptr(int n) {
    switch (n) {
    case 0: return &al;
    case 1: return &cl;
    case 2: return &dl;
    case 3: return &bl;
    case 4: return &ah;
    case 5: return &ch;
    case 6: return &dh;
    case 7: return &bh;
    default: LOG(FATAL) << "Invalid reg number encountered: " << n;
    }
    return &al;
  }
};

union regs_t {
  struct regs16 x;
  struct regs8 h;
};

struct sregs_t {
  uint16_t cs{0};
  uint16_t es{0};
  uint16_t ds{0};
  uint16_t fs{0};
  uint16_t gs{0};
  uint16_t ss{0};
  // ES = 0, CS = 1, SS = 2, DS = 3, FS = 4, and GS = 5.
  uint16_t get(int n) {
    switch (n) {
    case 0: return es;
    case 1: return cs;
    case 2: return ss;
    case 3: return ds;
    case 4: return fs;
    case 5: return gs;
    default: LOG(FATAL) << "Invalid reg number encountered: " << n;
    }
    return es;
  }

  void set(int n, uint16_t value) {
    switch (n) {
    case 0: es = value; break;
    case 1: cs = value; break;
    case 2: ss = value; break;
    case 3: ds = value; break;
    case 4: fs = value; break;
    case 5: gs = value; break;
    default: LOG(FATAL) << "Invalid reg number encountered: " << n;
    }
  }

  uint16_t get(segment_t n) { return get(static_cast<int>(n)); }

  uint16_t* regptr(int n) {
    switch (n) {
    case 0: return &es;
    case 1: return &cs;
    case 2: return &ss;
    case 3: return &ds;
    case 4: return &fs;
    case 5: return &gs;
    default: LOG(FATAL) << "Invalid reg number encountered: " << n;
    }
    return nullptr;
  }
};

constexpr uint16_t CF = 0x0001;
constexpr uint16_t PF = 0x0004;
constexpr uint16_t AF = 0x0010;
constexpr uint16_t ZF = 0x0040;
constexpr uint16_t SF = 0x0080;
constexpr uint16_t TF = 0x0100;
constexpr uint16_t IF = 0x0200;
constexpr uint16_t DF = 0x0400;
constexpr uint16_t OF = 0x0800;

#define FLAG_SET(flags, flg) ((flags) |= (flg))
#define FLAG_CLEAR(flags, flg) ((flags) &= ~(flg))
#define FLAG_FLIP(flags, flg) ((flags) ^= (flg))
#define FLAG_TEST(flags, flg) ((flags) & (flg))

// Use to set a value of a flag only if the condution 'cond' is true
#define FLAG_COND(cond, flags, flg)                                                                \
  do {                                                                                             \
    if (cond) {                                                                                    \
      FLAG_SET((flags.value_), (flg));                                                             \
    } else {                                                                                       \
      FLAG_CLEAR((flags.value_), (flg));                                                           \
    }                                                                                              \
  } while (0);

class flags_t {
public:
  inline bool cflag() const { return value_ & CF; }
  inline bool pflag() const { return value_ & PF; }
  inline bool aflag() const { return value_ & AF; }
  inline bool zflag() const { return value_ & ZF; }
  inline bool sflag() const { return value_ & ZF; }
  inline bool tflag() const { return value_ & TF; }
  inline bool iflag() const { return value_ & IF; }
  inline bool dflag() const { return value_ & DF; }
  inline bool oflag() const { return value_ & OF; }

  // set

  inline void cflag(bool b) {
    if (b)
      value_ |= CF;
    else
      value_ &= ~CF;
  }
  inline void pflag(bool b) {
    if (b)
      value_ |= PF;
    else
      value_ &= ~PF;
  }
  inline void aflag(bool b) {
    if (b)
      value_ |= AF;
    else
      value_ &= ~AF;
  }
  inline void zflag(bool b) {
    if (b)
      value_ |= ZF;
    else
      value_ &= ~ZF;
  }
  inline void sflag(bool b) {
    if (b)
      value_ |= SF;
    else
      value_ &= ~SF;
  }
  inline void tflag(bool b) {
    if (b)
      value_ |= TF;
    else
      value_ &= ~TF;
  }
  inline void iflag(bool b) {
    if (b)
      value_ |= IF;
    else
      value_ &= ~IF;
  }
  inline void dflag(bool b) {
    if (b)
      value_ |= DF;
    else
      value_ &= ~DF;
  }
  inline void oflag(bool b) {
    if (b)
      value_ |= OF;
    else
      value_ &= ~OF;
  }

  // bit 1 and 12-15 are always on
  inline void reset() { value_ = 0xf002; }
  inline void set(uint16_t flg) { value_ |= flg; }
  inline void clear(uint16_t flg) { value_ &= !flg; }
  inline bool test(uint16_t flg) const { return value_ & flg; }

  uint16_t value_{0xf002};
};

} // namespace door86::cpu::x86

#endif
