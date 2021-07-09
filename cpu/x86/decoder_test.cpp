#include <gtest/gtest.h>

#include "cpu/x86/cpu_fixture.h"
#include "cpu/x86/decoder.h"
#include <iostream>

using namespace door86::cpu::x86;

TEST(DecoderTest, OpsMatch) {
  Decoder decoder;
  for (uint8_t i = 0; i != 255; i++) {
    ASSERT_EQ(i, decoder.op_data(i).op);
  }
}

TEST(DecoderTest, ToString) {
  Decoder decoder;
  std::string ops("\xB8\x01\x00\x8E\xD8\xB4\x09\x8D\x16\x02\x00\xCD\x21", 13);
  uint8_t* ip = reinterpret_cast<uint8_t*>(ops.data());
  uint8_t* end = ip + ops.length();
  instruction_t inst{};
  int line = 0;
  while (ip < end) {
    auto inst = decoder.decode(ip);
    std::cout << ++line << ": " << inst.DebugString() << std::endl;
    ip += inst.len;
  }
}

TEST(DecoderTest, Smoke) {
  Decoder decoder;
  std::string ops("\xB8\x01\x00\x8E\xD8\xB4\x09\x8D\x16\x02\x00\xCD\x21", 13);
  uint8_t* ip = reinterpret_cast<uint8_t*>(ops.data());
  uint8_t* end = ip + ops.length();
  auto inst = decoder.decode(ip);
  ip += inst.len;
  EXPECT_EQ("MOV", inst.metadata.name);
  inst = decoder.decode(ip);
  ip += inst.len;
  EXPECT_EQ("MOV", inst.metadata.name);
  inst = decoder.decode(ip);
  ip += inst.len;
  EXPECT_EQ("MOV", inst.metadata.name);
  inst = decoder.decode(ip);
  ip += inst.len;
  EXPECT_EQ("LEA", inst.metadata.name);
  inst = decoder.decode(ip);
  ip += inst.len;
  EXPECT_EQ("INT", inst.metadata.name);
}

// 810626008000
TEST(DecoderTest, Disp16AndImm16) {
  Decoder decoder;
  const auto ops = parse_opcodes_from_line("810626008000");
  const auto inst = decoder.decode(ops);

  EXPECT_EQ(0x81, inst.op);
  EXPECT_EQ(0x0026, inst.disp16);
  EXPECT_EQ(0x0080, inst.imm16);
}

// 833E260000 - Disp16AndImm8