#include <gtest/gtest.h>

#include "cpu/x86/decoder.h"

using namespace door86::cpu::x86;

TEST(DecoderTest, OpsMatch) {
  for (int i = 0; i < 256; i++) {
    ASSERT_EQ(i, op_code_data[i].op);
  }
}
