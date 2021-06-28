#include <gtest/gtest.h>

#include "dos/psp.h"

#include <cstdlib>
#include <cstdint>
#include <iostream>

using namespace door86::dos;

/**
 * Check size is right and then sanity check just a couple of fields.
 */
TEST(PspTest, StructSize) { 
  ASSERT_EQ(256, sizeof(psp_t)); 
  EXPECT_EQ(0x18, offsetof(psp_t, job_file_table));
  EXPECT_EQ(0x40, offsetof(psp_t, dos_version_to_return));
}