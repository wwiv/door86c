#ifndef INCLUDED_CPU_X86_CORE_H
#define INCLUDED_CPU_X86_CORE_H

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
  uint16_t cflag;

  uint16_t& regref(int n) {
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
    // TODO(rushfan): GPF? Crash? What?
    return ax;
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
    }
    // TODO(rushfan): GPF? Crash? What?
    return nullptr;
  }

  void set(int n, uint16_t val) {
    switch (n) {
    case 0: ax = val;
    case 1: cx = val;
    case 2: dx = val;
    case 3: bx = val;
    case 4: sp = val;
    case 5: bp = val;
    case 6: si = val;
    case 7: di = val;
    }
  }
};

struct regs8 {
  uint8_t al, ah;
  uint8_t cl, ch;
  uint8_t dl, dh;
  uint8_t bl, bh;

  uint8_t& regref(int n) {
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
    // TODO(rushfan): GPF? Crash? What?
    return al;
  }

  void set(int n, uint8_t val) {
    switch (n) {
    case 0: al = val;
    case 1: cl = val;
    case 2: dl = val;
    case 3: bl = val;
    case 4: ah = val;
    case 5: ch = val;
    case 6: dh = val;
    case 7: bh = val;
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
    }
    // TODO(rushfan): GPF? Crash? What?
    return &al;
  }
};

union regs_t {
  struct regs16 x;
  struct regs8 h;
};

struct sregs_t {
  uint16_t cs;
  uint16_t es;
  uint16_t ds;
  uint16_t fs;
  uint16_t gs;
  uint16_t ss;
  // ES = 0, CS = 1, SS = 2, DS = 3, FS = 4, and GS = 5.
  uint16_t& regref(int n) {
    switch (n) {
    case 0: return es;
    case 1: return cs;
    case 2: return ss;
    case 3: return ds;
    case 4: return fs;
    case 5: return gs;
    }
    // TODO(rushfan): GPF? Crash? What?
    return es;
  }
  uint16_t regref(segment_t n) { return regref(static_cast<int>(n));
  }
};

constexpr int16_t CF = 0x0001;
constexpr int16_t PF = 0x0004;
constexpr int16_t AF = 0x0010;
constexpr int16_t ZF = 0x0040;
constexpr int16_t SF = 0x0080;
constexpr int16_t TF = 0x0100;
constexpr int16_t IF = 0x0200;
constexpr int16_t DF = 0x0400;
constexpr int16_t OF = 0x0800;

class flags_t {
public:
  bool cf() { return flag & CF; }
  // bit 1 and 12-15 are always on
  void reset() { flag = 0xf002;  } 

  uint16_t flag;
};

class cpu_core {
public:
  regs_t  regs;
  sregs_t sregs;
  flags_t flags;
  uint16_t ip;
};

class CPU {
public:
  CPU();

  // opcode executing

  bool execute(uint16_t cs, uint16_t ip);
  void execute_0x0(const instruction_t& inst);
  void execute_0x8(const instruction_t& inst);
  void execute_0xB(const instruction_t& inst);
  void execute_0xC(const instruction_t& inst);

  // stack handling

  void push(uint16_t val);
  uint16_t pop();

  // interrupt handling

  void call_interrupt(int num);

  cpu_core core;
  Decoder decoder;
  Memory memory;

private:
  bool running_{true};
};

template<typename T> class Rmm {
public:
  Rmm(T* reg) : mem_(nullptr), reg_(reg), seg_(0), off_(0) {}
  Rmm(Memory* mem, uint16_t seg, uint16_t off) : mem_(mem), reg_(nullptr), seg_(seg), off_(off) {}

  T get() const {
    if (mem_) {
      return mem_->get<T>(seg_, off_);
    } 
    return *reg_;
  }

  void set(T v) {
    if (mem_) {
      mem_->set<T>(seg_, off_, v);
    } else {
      *reg_ = v;
    }
  }

  Rmm& operator+=(const T& other) { 
    T cur = get();
    cur += other;
    set(cur);
    return *this;
  }

private:
  Memory* mem_;
  T* reg_;
  uint16_t seg_;
  uint16_t off_;
};

}
#endif