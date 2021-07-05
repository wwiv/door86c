#include <gtest/gtest.h>

#include "cpu/x86/decoder.h"
#include "cpu/memory.h"
#include "cpu/x86/cpu.h"
#include "cpu/x86/rmm.h"
#include <iostream>

using namespace door86::cpu;
using namespace door86::cpu::x86;

class RmmTest : public testing::Test {
public:
  RmmTest() { }

  Rmm<RmmType::REGISTER, uint16_t> ax() {
    return Rmm<RmmType::REGISTER, uint16_t>(&core, &core.regs.x.ax);
  };
  Rmm<RmmType::REGISTER, uint16_t> bx() {
    return Rmm<RmmType::REGISTER, uint16_t>(&core, &core.regs.x.bx);
  };
  Rmm<RmmType::REGISTER, uint16_t> cx() {
    return Rmm<RmmType::REGISTER, uint16_t>(&core, &core.regs.x.cx);
  };
  Rmm<RmmType::REGISTER, uint8_t> ch() {
    return Rmm<RmmType::REGISTER, uint8_t>(&core, &core.regs.h.ch);
  };
  Rmm<RmmType::REGISTER, uint16_t> dx() {
    return Rmm<RmmType::REGISTER, uint16_t>(&core, &core.regs.x.dx);
  };
  Rmm<RmmType::MEMORY, uint16_t> mem16(uint16_t seg, uint16_t off, uint16_t val) {
    memory.set<uint16_t>(seg, off, val);
    return Rmm<RmmType::MEMORY, uint16_t>(&core, &memory, seg, off);
  }
  Rmm<RmmType::MEMORY, uint8_t> mem8(uint16_t seg, uint16_t off, uint8_t val) {
    memory.set<uint8_t>(seg, off, val);
    return Rmm<RmmType::MEMORY, uint8_t>(&core, &memory, seg, off);
  }

  cpu_core core;
  Decoder decoder;
  Memory memory{1 << 20};
};

TEST_F(RmmTest, MemoryAccess) { 
  // cpu_core* core, Memory* mem, uint16_t seg, uint16_t off
  auto rmm = mem16(10, 0, 0xcafe);
  ASSERT_EQ(0xCAFE, rmm.get());
}

TEST_F(RmmTest, RegisterAccess) {
  // cpu_core* core, Memory* mem, uint16_t seg, uint16_t off
  core.regs.x.cx = 0xCAFE;
  auto rmm = cx();
  ASSERT_EQ(0xCAFE, rmm.get());
}

TEST_F(RmmTest, Reg_CF) { 
  EXPECT_FALSE(core.flags.cflag());
  core.regs.x.cx = 0xffff;
  auto r = cx(); 
  r += 1;
  EXPECT_EQ(0x0000, r.get());
  EXPECT_TRUE(core.flags.cflag());
  EXPECT_TRUE(core.flags.zflag());
}

TEST_F(RmmTest, Reg_ZF) {
  EXPECT_FALSE(core.flags.cflag());
  core.regs.x.cx = 0xffaa;
  auto r = cx();
  r -= 0xffaa;
  EXPECT_EQ(0x0000, r.get());
  EXPECT_FALSE(core.flags.cflag());
  EXPECT_TRUE(core.flags.zflag());
}

TEST_F(RmmTest, SHL8) {
  EXPECT_FALSE(core.flags.cflag());
  core.regs.h.ch = 0xff;
  auto r = ch();
  EXPECT_EQ(0xff, r.get());
  r.shl(1);
  EXPECT_EQ(0xfe, r.get());
  EXPECT_TRUE(core.flags.cflag());
}

TEST_F(RmmTest, SHR8) {
  EXPECT_FALSE(core.flags.cflag());
  core.regs.h.ch = 0xff;
  auto r = ch();
  EXPECT_EQ(0xff, r.get());
  r.shr(1);
  EXPECT_EQ(0x7f, r.get());
  EXPECT_TRUE(core.flags.cflag());
}

TEST_F(RmmTest, SHR16) {
  EXPECT_FALSE(core.flags.cflag());
  core.regs.x.cx = 0xffff;
  auto r = cx();
  EXPECT_EQ(0xffff, r.get());
  r.shr(1);
  EXPECT_EQ(0x7fff, r.get());
  EXPECT_TRUE(core.flags.cflag());
}

TEST_F(RmmTest, SAR8) {
  EXPECT_FALSE(core.flags.cflag());
  core.regs.h.ch = 0xff;
  auto r = ch();
  EXPECT_EQ(0xff, r.get());
  r.sar(1);
  EXPECT_EQ(0xbf, r.get()); // 0xbf is (0x7f >> 1 | 0x80)
  EXPECT_TRUE(core.flags.cflag());
}

TEST_F(RmmTest, SAR16) {
  EXPECT_FALSE(core.flags.cflag());
  core.regs.x.cx = 0xffff;
  auto r = cx();
  EXPECT_EQ(0xffff, r.get());
  r.sar(1);
  EXPECT_EQ(0xbfff, r.get()); // 0xbfff is (0x7fff >> 1 | 0x80)
  EXPECT_TRUE(core.flags.cflag());
}

TEST_F(RmmTest, ROL8) {
  EXPECT_FALSE(core.flags.cflag());
  core.regs.h.ch = 0x81;
  auto r = ch();
  EXPECT_EQ(0x81, r.get());
  r.rol(1);
  EXPECT_EQ(0x03, r.get());
  EXPECT_TRUE(core.flags.cflag());
}

TEST_F(RmmTest, RCL8) {
  EXPECT_FALSE(core.flags.cflag());
  core.regs.h.ch = 0x81;
  auto r = ch();
  r.rcl(1);
  EXPECT_EQ(0x02, r.get());
  EXPECT_TRUE(core.flags.cflag());
  r.rcl(1);
  // bit 4 + carry flag
  EXPECT_EQ(0x05, r.get());
  EXPECT_FALSE(core.flags.cflag());
}

TEST_F(RmmTest, RCR8) {
  EXPECT_FALSE(core.flags.cflag());
  core.regs.h.ch = 0x11;
  auto r = ch();
  r.rcr(1);
  EXPECT_EQ(0x08, r.get());
  EXPECT_TRUE(core.flags.cflag());
}

TEST_F(RmmTest, XCHG) { 
  core.regs.h.ch = 0x11;
  auto r = ch();
  auto rm = mem8(10, 10, 0x22);
  swap(r, rm);
  EXPECT_EQ(core.regs.h.ch, 0x22);
  EXPECT_EQ(memory.get<uint8_t>(10, 10), 0x11);
}