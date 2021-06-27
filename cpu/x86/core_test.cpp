#include <gtest/gtest.h>

#include "cpu/x86/core.h"
#include "cpu/x86/decoder.h"
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

