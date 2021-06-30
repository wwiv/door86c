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
  EXPECT_EQ(256, sizeof(psp_t)); 
  EXPECT_EQ(0x05, offsetof(psp_t, call_to_dos_fn_dispatcher));
  EXPECT_EQ(0x0e, offsetof(psp_t, addr_break_handler));
  EXPECT_EQ(0x12, offsetof(psp_t, addr_crit_error_handler));
  EXPECT_EQ(0x16, offsetof(psp_t, parent_psp_segment));
  EXPECT_EQ(0x18, offsetof(psp_t, job_file_table));
  EXPECT_EQ(0x2c, offsetof(psp_t, environ_seg));
  EXPECT_EQ(0x32, offsetof(psp_t, jft_size));
  EXPECT_EQ(0x40, offsetof(psp_t, dos_version_to_return));
  EXPECT_EQ(0x50, offsetof(psp_t, int21_retf_instructions));
  EXPECT_EQ(0x5c, offsetof(psp_t, fcb1));
  EXPECT_EQ(0x6c, offsetof(psp_t, fcb2));
  EXPECT_EQ(0x80, offsetof(psp_t, cmdlen_length));
  EXPECT_EQ(0x81, offsetof(psp_t, cmdline));
}
