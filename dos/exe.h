#ifndef INCLUDED_EXE_PSP_H
#define INCLUDED_EXE_PSP_H

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace door86::dos {
struct exe_header_t {
  uint16_t signature; /* == 0x5a4D */
  uint16_t bytes_in_last_block;
  uint16_t blocks_in_file;
  uint16_t num_relocs;
  uint16_t header_paragraphs;
  uint16_t min_extra_paragraphs;
  uint16_t max_extra_paragraphs;
  uint16_t ss;
  uint16_t sp;
  uint16_t checksum;
  uint16_t ip;
  uint16_t cs;
  uint16_t reloc_table_offset;
  uint16_t overlay_number;
};

struct exe_reloc_table_entry_t {
  uint16_t offset;
  uint16_t segment;
};

struct exe_info_t {
  exe_header_t hdr;
  std::vector<exe_reloc_table_entry_t> relos;
  int binary_size;
};

std::optional<exe_info_t> read_exe_header(FILE* fp, const std::string& filename);

}

#endif  // INCLUDED_EXE_PSP_H