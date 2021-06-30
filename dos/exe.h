#ifndef INCLUDED_DOS_PSP_H
#define INCLUDED_DOS_PSP_H

#include "cpu/memory.h"

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace door86::dos {
struct exe_header_t {
  uint16_t signature; /* == 0x5a4D */
  uint16_t bytes_in_last_block{0};
  uint16_t blocks_in_file{0};
  uint16_t num_relocs{0};
  uint16_t header_paragraphs{0};
  uint16_t min_extra_paragraphs{0};
  uint16_t max_extra_paragraphs{0};
  uint16_t ss{0};
  uint16_t sp{0};
  uint16_t checksum{0};
  uint16_t ip{0};
  uint16_t cs{0};
  uint16_t reloc_table_offset{0};
  uint16_t overlay_number{0};
};

struct exe_reloc_table_entry_t {
  uint16_t offset;
  uint16_t segment;
};

class Exe {
public:
  uint32_t header_size() const { return hdr.header_paragraphs * 0x10; }
  // Calls load image on the exe specified
  bool load_image(uint16_t base_segment, door86::cpu::Memory& mem);

  // data
  std::filesystem::path filepath;
  exe_header_t hdr{};
  std::vector<exe_reloc_table_entry_t> relos;
  int binary_size{0};

  // set once loaded.
  bool loaded_{false};
  // this is where the PSP will be located
  uint16_t seg;
  // Where the loaded image will be located, this is 256 bytes after the seg (which
  // has the PSP)
  uint16_t image_seg;

};

std::optional<Exe> read_exe_header(const std::filesystem::path& filepath);

// Calls load image on the com file located at filepath
bool load_image(const std::filesystem::path& filepath, uint16_t base_segment,
                door86::cpu::Memory& mem);

} // namespace door86::dos

#endif // INCLUDED_DOS_PSP_H