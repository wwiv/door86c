#include <gtest/gtest.h>

#include "cpu/x86/decoder.h"
#include "cpu/memory.h"
#include "cpu/x86/core.h"
#include "cpu/x86/rmm.h"
#include <iostream>

using namespace door86::cpu;
using namespace door86::cpu::x86;

class RmmTest : public testing::Test {
public:
  RmmTest() { }

  Rmm<uint16_t> ax() { return Rmm<uint16_t>(&core, &core.regs.x.ax); };
  Rmm<uint16_t> bx() { return Rmm<uint16_t>(&core, &core.regs.x.bx); };
  Rmm<uint16_t> cx() { return Rmm<uint16_t>(&core, &core.regs.x.cx); };
  Rmm<uint16_t> dx() { return Rmm<uint16_t>(&core, &core.regs.x.dx); };

  cpu_core core;
  Decoder decoder;
  Memory memory{1 << 20};
};

TEST_F(RmmTest, MemoryAccess) { 
  // cpu_core* core, Memory* mem, uint16_t seg, uint16_t off
  memory.set<uint16_t>(10, 0, 0xcafe);
  Rmm<uint16_t> rmm(&core, &memory, 10, 0);
  ASSERT_EQ(0xCAFE, rmm.get());
}

TEST_F(RmmTest, RegisterAccess) {
  // cpu_core* core, Memory* mem, uint16_t seg, uint16_t off
  core.regs.x.cx = 0xCAFE;
  Rmm<uint16_t> rmm(&core, &core.regs.x.cx);
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
