#include <gtest/gtest.h>

#include "cpu/x86/core.h"
#include "cpu/x86/decoder.h"
#include <iostream>

using namespace door86::cpu::x86;

TEST(CPUTest, Smoke) { 
  std::string ops("\xB8\x01\x00\x8E\xD8\xB4\x09\x8D\x16\x02\x00\xCD\x21", 13);
  
  CPU c;
  ASSERT_TRUE(c.memory.load_image(0x10, 13, reinterpret_cast<uint8_t*>(ops.data())));
  EXPECT_TRUE(c.execute(0, 0x10));
}

