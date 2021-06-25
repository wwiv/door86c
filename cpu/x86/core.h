#ifndef INCLUDED_CPU_X86_CORE_H
#define INCLUDED_CPU_X86_CORE_H

#include <cstdint>

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
  uint16_t cflag;

  uint16_t& by_reg(int n) {
    switch (n) {
    case 0: return ax;
    case 1: return cx;
    case 2: return dx;
    case 3: return bx;
    case 4: return sp;
    case 5: return bp;
    case 6: return si;
    case 7: return di;
    }
  }
};

struct regs8 {
  uint8_t al, ah;
  uint8_t cl, ch;
  uint8_t dl, dh;
  uint8_t bl, bh;
  uint8_t& by_reg(int n) {
    switch (n) {
    case 0: return al;
    case 1: return cl;
    case 2: return dl;
    case 3: return bl;
    case 4: return ah;
    case 5: return ch;
    case 6: return dh;
    case 7: return bh;
    }
  }
};

union regs_t {
  struct regs16 x;
  struct regs8 h;
};

struct sregs_t {
  uint16_t es;
  uint16_t cs;
  uint16_t ss;
  uint16_t ds;
};

struct flags_t {
  bool carry{ false };

};

struct reg_mod_rm {
  // XX000000
  uint8_t mod;
  // 00XXX000
  uint8_t reg;
  // 000000XX
  uint8_t rm;
};

struct cpu_core {
  regs_t  regs;
  sregs_t sregs;
  flags_t flags;
};

class CPU {
public:
  CPU();

private:
  cpu_core core_;
};

}
#endif