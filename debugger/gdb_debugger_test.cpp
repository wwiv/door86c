#include <gtest/gtest.h>

#include "debugger/debugger.h"
#include "debugger/gdb_debugger.h"
#include <cstdint>
#include <iostream>

using namespace door86::dbg;

TEST(GdbTest, Smoke) { 
  ASSERT_TRUE(validate_checksum("A", "41"));
  ASSERT_TRUE(validate_checksum("", "00"));
}

TEST(GdbTest, Invalid) {
  ASSERT_FALSE(validate_checksum("FOO", "41"));
}
