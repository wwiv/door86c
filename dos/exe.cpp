#include "dos/exe.h"

#include "fmt/format.h"
#include <cstdio>
#include <string>
#include <optional>

namespace door86::dos {

std::optional<exe_info_t> read_exe_header(FILE* fp, const std::string& filename) {
  exe_info_t info;

  if (fseek(fp, 0, SEEK_SET) != 0) {
    fmt::print("Unable to seek to relo offsets from: [%s]\r\n", filename);
    return std::nullopt;
  }
  if (const auto num_read = fread(&info.hdr, sizeof(exe_header_t), 1, fp); num_read != 1) {
    fmt::print("Unable to read header from: [%s]\r\n", filename);
    return std::nullopt;
  }
  info.binary_size = ((info.hdr.blocks_in_file * 512) + info.hdr.bytes_in_last_block);
  if (info.hdr.bytes_in_last_block) {
    info.binary_size -= 512;
  }
  if (fseek(fp, info.hdr.reloc_table_offset, SEEK_SET) != 0) {
    fmt::print("Unable to seek to relo offsets from: [%s]\r\n", filename);
    return std::nullopt;
  }

  info.relos.resize(info.hdr.num_relocs);
  if (fread(&info.relos[0], sizeof(exe_reloc_table_entry_t), info.hdr.num_relocs, fp) !=
      info.hdr.num_relocs) {
    fmt::print("Unable to read relo offsets from: [%s]\r\n", filename);
    return std::nullopt;
  }

  return info;
}

}

