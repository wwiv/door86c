#include <gtest/gtest.h>
#include "gmock/gmock.h"

#include "cpu/x86/core.h"
#include "cpu/x86/decoder.h"
#include "cpu/x86/cpu_fixture.h"
#include <iostream>

using namespace door86::cpu::x86;

/*
00000000  B80100            mov ax,0x1
00000003  8ED8              mov ds,ax
00000005  B409              mov ah,0x9
00000007  8D160200          lea dx,[0x2]
0000000B  CD21              int 0x21
0000000D  B44C              mov ah,0x4c
0000000F  CD21              int 0x21
00000011  004865            add [bx+si+0x65],cl
00000014  6C                insb
00000015  6C                insb
00000016  6F                outsw
00000017  2C20              sub al,0x20
00000019  7468              jz 0x83
0000001B  657265            gs jc 0x83
0000001E  2E0D0A24          cs or ax,0x240a 
 */
TEST(CPUTest, Smoke) { 
  const auto len = 34;
  std::string ops("\xB8\x01\x00\x8E\xD8\xB4\x09\x8D"
                  "\x16\x02\x00\xCD\x21\xB4\x4C\xCD"
                  "\x21\x00\x48\x65\x6C\x6C\x6F\x2C"
                  "\x20\x74\x68\x65\x72\x65\x2E\x0D"
                  "\x0A\x24", len);
  
  CPU c;
  // load at 0 since there is no fixups like DOS does when loading the image.
  ASSERT_TRUE(c.memory.load_image(0x0, len, reinterpret_cast<uint8_t*>(ops.data())));
  EXPECT_TRUE(c.execute(0, 0x0));
}


TEST(CPUTest, HelloThere1) {
  constexpr char* s = R"(
00000030  B8 02 00 8E D8 B4 09 BB 10 00 83 C3 01 83 C3 01   ................
00000040  83 C3 01 83 C3 01 8D 16 00 00 CD 21 B4 4C CD 21   ...........!.L.!
00000050  48 65 6C 6C 6F 2C 20 74 68 65 72 65 2E 0D 0A 24   Hello, there...$
)";
  auto ops = parse_opcodes_from_textdump(s); 

  CPU c;
  // load at 0 since there is no fixups like DOS does when loading the image.
  ASSERT_TRUE(c.memory.load_image(0x0, ops.size(), ops.data()));
  EXPECT_TRUE(c.execute(0, 0x0));
}

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

TEST(CPUTest, Mov) {
  //B8FECA
  auto ops = parse_opcodes_from_line("B8FECA");
  CPU c;
  ASSERT_TRUE(c.memory.load_image(0x0, ops.size(), ops.data()));
  EXPECT_TRUE(c.execute(0, 0x0));
  EXPECT_EQ(0xCAFE, c.core.regs.x.ax);
}