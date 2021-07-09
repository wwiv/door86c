#include <gtest/gtest.h>

#include "cpu/memory.h"
#include "dos/dos.h"

#include <cstdlib>
#include <cstdint>
#include <iostream>

using namespace door86::cpu;
using namespace door86::dos;

class DosMemMgrTest : public testing::Test {
public:
  DosMemMgrTest() : mm(&m, 0x1000, 0x7000) {}

  Memory m{1 << 20};
  DosMemoryManager mm;
};

TEST_F(DosMemMgrTest, Smoke) { 
  ASSERT_EQ(1, mm.blocks().size());
  const auto o = mm.allocate(0x2000);
  ASSERT_TRUE(o.has_value());
  ASSERT_EQ(0x1000, o.value());

  const auto o2 = mm.allocate(0x5001);
  ASSERT_FALSE(o2.has_value());
}

TEST_F(DosMemMgrTest, Tail) {
  mm.strategy(DosMemoryManager::fit_strategy_t::last);
  ASSERT_EQ(1, mm.blocks().size());
  const auto o = mm.allocate(0x2000);
  ASSERT_TRUE(o.has_value());
  ASSERT_EQ(0x5000, o.value());

  const auto o2 = mm.allocate(0x5001);
  ASSERT_FALSE(o2.has_value());
}
