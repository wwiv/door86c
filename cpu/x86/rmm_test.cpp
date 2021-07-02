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


