#include <gtest/gtest.h>

#include "cpu/memory.h"
#include <cstdint>
#include <iostream>

using namespace door86::cpu;

TEST(MemoryTest, Smoke) {
  Memory m(100);
  uint8_t img[] = {0xad, 0xde, 0xef, 0xbe, 0x00, 0x00};

  ASSERT_TRUE(m.load_image(0, 6, img));
  EXPECT_EQ(m[0], 0xad);
  EXPECT_EQ(m[1], 0xde);
}

TEST(MemoryTest, ImageTooBig) {
  Memory m(10);
  uint8_t img[] = {0xad, 0xde, 0xef, 0xbe, 0x00, 0x00, 0xad, 0xde, 0xef, 0xbe, 0x00, 0x00};

  ASSERT_FALSE(m.load_image(0, 12, img));
  EXPECT_NE(m[0], 0xad);
  EXPECT_NE(m[1], 0xde);
}

TEST(MemoryTest, Word) { 
  Memory m(100);
  uint8_t img[] = {0xad, 0xde, 0xef, 0xbe, 0x00, 0x00};

  ASSERT_TRUE(m.load_image(0, 6, img));

  uint16_t* w = m.word(0, 0);
  EXPECT_EQ(*w, 0xdead);
   w = m.word(0, 2);
  EXPECT_EQ(*w, 0xbeef);

  uint16_t& r = m.wordref(0, 0);
  EXPECT_EQ(r, 0xdead);
  r = m.wordref(0, 2);
  EXPECT_EQ(r, 0xbeef);
}

TEST(MemoryTest, MutateByte) {
  Memory m(100);
  uint8_t img[] = {0xad, 0xde, 0xef, 0xbe, 0x00, 0x00};

  ASSERT_TRUE(m.load_image(0, 6, img));
  
  auto& l = m[0];
  ASSERT_EQ(l, 0xad);
  l = 0xfe;
  ASSERT_EQ(m[0], 0xfe);
}

TEST(MemoryTest, MutateWord) {
  Memory m(100);
  uint8_t img[] = {0xad, 0xde, 0xef, 0xbe, 0x00, 0x00};

  ASSERT_TRUE(m.load_image(0, 6, img));

  uint16_t& r = m.wordref(0, 0);
  ASSERT_EQ(r, 0xdead);

  r = 0xabcd;
  ASSERT_EQ(m[0], 0xcd);
  ASSERT_EQ(m[1], 0xab);
}
