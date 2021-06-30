#include "dos/exe.h"

#include "core/file.h"
#include "core/log.h"
#include "core/scope_exit.h"
#include "fmt/format.h"
#include <cstdio>
#include <string>
#include <optional>

// MSVC only has __PRETTY_FUNCTION__ in intellisense, 
// TODO(rushfan): Find a better home for this macro.
#if !defined(__PRETTY_FUNCTION__)
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

namespace door86::dos {

using namespace wwiv::core;

bool is_exe(const std::filesystem::path& filepath) {
  FILE* fp = nullptr;
  if (fp = fopen(filepath.string().c_str(), "rb"); !fp) {
    VLOG(1) << "Unable to open file: " << filepath;
    return false;
  }
  // Close fp at exit, no matter how
  ScopeExit at_exit([=] { fclose(fp); });

  char mz[2];
  if (const auto num_read = fread(&mz, 1, 2, fp); num_read != 2) {
    VLOG(1) << "Unable to seek on file: " << filepath;
    return false;
  }

  return mz[0] == 'M' && mz[1] == 'Z';
}

std::optional<Exe> read_exe_header(const std::filesystem::path& filepath) {
  VLOG(4) << __PRETTY_FUNCTION__ << ": " << filepath.string();
  if (!File::Exists(filepath)) {
    VLOG(1) << "read_exe_header: file not found: " << filepath.string();
    return std::nullopt;
  }
  const auto filename = filepath.filename().string();

  Exe exe{};
  exe.filepath = filepath;

  FILE* fp = nullptr;
  if (fp = fopen(filepath.string().c_str(), "rb"); !fp) {
    VLOG(1) << "Unable to open file: " << filename;
    return std::nullopt;
  }
  // Close fp at exit, no matter how
  ScopeExit at_exit([=] { fclose(fp); });

  char mz[2];
  if (const auto num_read = fread(&mz, 1, 2, fp); num_read != 2) {
    VLOG(1) << "Unable to seek on file: " << filename;
    return std::nullopt;
  }

  if (mz[0] != 'M' && mz[1] != 'Z') {
    VLOG(1) << "Not an exe: " << filename;
    return std::nullopt;
  }

  if (fseek(fp, 0, SEEK_SET) != 0) {
    VLOG(1) << "Unable to seek to relo offsets on file: " << filename;
    return std::nullopt;
  }
  if (const auto num_read = fread(&exe.hdr, sizeof(exe_header_t), 1, fp); num_read != 1) {
    VLOG(1) << "Unable to read header from file: " << filename;
    return std::nullopt;
  }
  exe.binary_size = ((exe.hdr.blocks_in_file * 512) + exe.hdr.bytes_in_last_block);
  if (exe.hdr.bytes_in_last_block) {
    exe.binary_size -= 512;
  }
  if (fseek(fp, exe.hdr.reloc_table_offset, SEEK_SET) != 0) {
    fmt::print("Unable to seek to relo offsets from: [%s]\r\n", filename);
    return std::nullopt;
  }

  exe.relos.resize(exe.hdr.num_relocs);
  if (fread(&exe.relos[0], sizeof(exe_reloc_table_entry_t), exe.hdr.num_relocs, fp) !=
      exe.hdr.num_relocs) {
    fmt::print("Unable to read relo offsets from: [%s]\r\n", filename);
    return std::nullopt;
  }

  return exe;
}

bool Exe::load_image(uint16_t base_segment, door86::cpu::Memory& mem) {
  seg = base_segment;
  image_seg = base_segment + 0x10;

  if (!door86::dos::load_image(filepath, image_seg, header_size(), mem)) {
    return false;
  }
  loaded_ = true;
  // fixup relo offsets
  for (const auto& relo : relos) {
    VLOG(1) << "Relocating offset at: " << relo.segment << ":" << relo.offset;
    const uint32_t addr = (relo.segment * 0x10) + (image_seg * 0x10) + relo.offset;
    auto relo_seg = mem.abs16(addr);
    relo_seg += image_seg;
    mem.abs16(addr, relo_seg);
  }
  return true;
}

bool load_image(const std::filesystem::path& filepath, uint16_t base_segment, uint32_t offset,
                door86::cpu::Memory& mem) {
  if (!File::Exists(filepath)) {
    return false;
  }

  File f(filepath);
  if (!f.Open(File::modeBinary | File::modeReadOnly)) {
    return false;
  }

  const auto filesize = f.length() - offset;
  if (offset) {
    // skip header
    f.Seek(offset, File::Whence::begin);
  }
  if (f.Read(&mem[base_segment * 0x10], filesize) != filesize) {
    VLOG(1) << "Failed to read binary into memory";
  }
  return true;
}

}

