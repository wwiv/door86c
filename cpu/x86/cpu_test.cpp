#include <gtest/gtest.h>
#include "gmock/gmock.h"

#include "cpu/x86/cpu.h"
#include "cpu/x86/decoder.h"
#include "cpu/x86/cpu_fixture.h"
#include <iostream>

using namespace door86::cpu;
using namespace door86::cpu::x86;

class CPUTest : public testing::Test {
public:
  CPUTest() {}

  bool load(uint16_t seg, const std::vector<uint8_t>& ops) {
    const seg_address_t addr{seg, 0x0};
    return c.memory.load_image(seg, ops.size(), ops.data());
  }

  bool load(uint16_t seg, const std::string& s) {
    const seg_address_t addr{seg, 0x0};
    const auto ops = parse_opcodes_from_line(s);
    return c.memory.load_image(seg, ops.size(), ops.data());
  }

  bool load_string(uint16_t seg, const std::string& s) {
    const seg_address_t addr{seg, 0x0};
    return c.memory.load_string(seg * 0x10, s);
  }

  CPU c;
  Decoder decoder;
  Memory& mem{c.memory};
  regs_t& regs{c.core.regs};
  sregs_t& sregs{c.core.sregs};
  flags_t& flags{c.core.flags};
};

#if 0
TEST_F(CPUTest, HelloThere1) {
  constexpr char* s = R"(
00000030  B8 02 00 8E D8 B4 09 BB 10 00 83 C3 01 83 C3 01   ................
00000040  83 C3 01 83 C3 01 8D 16 00 00 CD 21 B4 4C CD 21   ...........!.L.!
00000050  48 65 6C 6C 6F 2C 20 74 68 65 72 65 2E 0D 0A 24   Hello, there...$
)";
  auto ops = parse_opcodes_from_textdump(s); 

  // load at 0 since there is no fixups like DOS does when loading the image.
  ASSERT_TRUE(c.memory.load_image(0x0, ops.size(), ops.data()));
  EXPECT_TRUE(c.run(0, 0x0));
}
#endif

TEST(CpuFixtureTest, Dump) { 
  constexpr char* s = R"(
00000030  B8 02 00 8E D8 B4 09 BB 10 00 83 C3 01 83 C3 01   ................
00000040  83 C3 01 83 C3 01 8D 16 00 00 CD 21 B4 4C CD 21   ...........!.L.!
00000050  48 65 6C 6C 6F 2C 20 74 68 65 72 65 2E 0D 0A 24   Hello, there...$
)";
  auto ops = parse_opcodes_from_textdump(s); 
  ASSERT_EQ(48u, ops.size());
  ASSERT_THAT(ops, ::testing::ElementsAre(
                       0xB8, 0x02, 0x00, 0x8E, 0xD8, 0xB4, 0x09, 0xBB, 0x10, 0x00, 0x83, 0xC3, 0x01,
                       0x83, 0xC3, 0x01, 0x83, 0xC3, 0x01, 0x83, 0xC3, 0x01, 0x8D, 0x16, 0x00, 0x00,
                       0xCD, 0x21, 0xB4, 0x4C, 0xCD, 0x21, 0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x2C, 0x20,
                       0x74, 0x68, 0x65, 0x72, 0x65, 0x2E, 0x0D, 0x0A, 0x24));
}

// Start tests for OPcodes here.

TEST_F(CPUTest, Mov) {
  auto ops = parse_opcodes_from_line("B8FECA");
  sregs.cs = 0;
  c.core.ip = 0;
  // execute single instruction.
  const auto inst = c.decoder.decode(ops);
  EXPECT_TRUE(c.execute(inst));
  EXPECT_EQ(0xCAFE, c.core.regs.x.ax);
  EXPECT_EQ(0xCAFE, regs.x.ax);
}

TEST_F(CPUTest, REPNE_SCAS) {
  sregs.es = 0x2000;
  regs.x.di = 0;
  mem.clear(0x20000, 100);
  mem.load_string(0x20000, "Hello World");
  const auto ops = parse_opcodes_from_line("F3 AE");
  const auto inst = c.decoder.decode(ops);
  regs.x.ax = 0x0000;
  regs.x.cx = 0xffff;
  auto r = c.execute(inst);
  regs.x.cx = ~regs.x.cx;
  EXPECT_EQ(12, regs.x.cx);
}
