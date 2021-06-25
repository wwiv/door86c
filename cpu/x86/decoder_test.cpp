#include <gtest/gtest.h>

#include "cpu/x86/decoder.h"
#include <iostream>

using namespace door86::cpu::x86;

TEST(DecoderTest, OpsMatch) {
  for (int i = 0; i < 256; i++) {
    ASSERT_EQ(i, op_code_data[i].op);
  }
}

TEST(DecoderTest, Smoke) {
  std::string ops("\xB8\x01\x00\x8E\xD8\xB4\x09\x8D\x16\x02\x00\xCD\x21", 13);
  uint8_t* ip = reinterpret_cast<uint8_t*>(ops.data());
  uint8_t* end = ip + ops.length();
  instruction_t inst{};
  int line = 0;
  while (ip < end) {
    auto inst = next_instruction(ip);
    std::cout << ++line << ": " << to_string(inst) << std::endl;
    ip += inst.len;
  }
}
