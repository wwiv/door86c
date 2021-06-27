#include <cstdio>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include "core/log.h"
#include "core/command_line.h"
#include "core/scope_exit.h"
#include "core/version.h"
#include "fmt/format.h"

using namespace wwiv::core;

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

static std::string to_seg_off(uint16_t seg, uint16_t off) {
  return fmt::format("{:04x}:{:04x}", seg, off);
}

static std::string exe_signature(uint16_t sig) {
  char s[3];
  s[0] = sig & 0xff;
  s[1] = (sig >> 8) & 0xff;
  s[2] = 0;
  return s;
}

int main(int argc, char** argv) {
  LoggerConfig config;
  Logger::Init(argc, argv, config);

  ScopeExit at_exit(Logger::ExitLogger);
  CommandLine cmdline(argc, argv, "");
  cmdline.add_argument(BooleanCommandLineArgument{"version", 'V', "Display version.", false});
  cmdline.set_no_args_allowed(true);

  if (!cmdline.Parse()) {
    std::cout << cmdline.GetHelp() << std::endl;
    return EXIT_FAILURE;
  }
  if (cmdline.help_requested()) {
    std::cout << cmdline.GetHelp() << std::endl;
    return EXIT_SUCCESS;
  }
  if (cmdline.barg("version")) {
    std::cout << full_version() << std::endl;
    return 0;
  }

  FILE* fp = nullptr;
  if (cmdline.remaining().empty()) {
    fprintf(stderr, "Usage: dis <exename>\r\n");
    return 1;
  }
  const auto& filename = cmdline.remaining().front();
  if (fp = fopen(filename.c_str(), "rb"); !fp) {
    fmt::print("Unable to open file: [%s]\r\n", filename);
    return 1;
  }

  exe_header_t hdr{};
  if (const auto num_read = fread(&hdr, sizeof(exe_header_t), 1, fp); num_read != 1) {
    fmt::print("Unable to read header from: [%s]\r\n", filename);
    return 1;
  }
  std::cout << "sizeof(exe_header_t)       " << sizeof(exe_header_t) << std::endl;
  int binary_size = ((hdr.blocks_in_file * 512) + hdr.bytes_in_last_block);
  if (hdr.bytes_in_last_block) {
    binary_size -= 512;
  }
  std::cout << "binary size:               " << binary_size << std::endl;
  std::cout << "num_relocs:                " << hdr.num_relocs << std::endl;
  fmt::print("EXE Header:                {}\r\n", exe_signature(hdr.signature));
  std::cout << "Bytes on last page :       " << hdr.bytes_in_last_block << std::endl;
  std::cout << "Pages in file :            " << hdr.blocks_in_file << std::endl;
  std::cout << "Relocations :              " << hdr.num_relocs << std::endl;
  std::cout << "Paragraphs in header :     " << hdr.header_paragraphs << std::endl;
  std::cout << "Header Size :              " << (16 * hdr.header_paragraphs) << std::endl;
  std::cout << "Extra paragraphs needed :  " << hdr.min_extra_paragraphs << std::endl;
  std::cout << "Extra paragraphs wanted :  " << hdr.max_extra_paragraphs << std::endl;
  std::cout << "Initial stack location :   " << to_seg_off(hdr.ss, hdr.sp) << std::endl;
  std::cout << "SP:                        0x" << std::hex << hdr.sp << std::endl;
  std::cout << "Word checksum :            0x" << std::hex << hdr.checksum << std::endl;
  std::cout << "Entry point :              " << hdr.ip << std::endl;
  std::cout << "Relocation table address : 0x" << std::hex << hdr.reloc_table_offset << std::endl;
  std::cout << "Memory needed :            ???" << std::endl;// << std::hex << hdr.
  std::cout << "CS:                        0x" << std::hex << hdr.cs << std::endl;
  std::cout << "Entry point:               " << (hdr.cs * 16) + hdr.ip << std::endl;
  std::cout << "\r\n\n";
  
  
  if (fseek(fp, hdr.reloc_table_offset, SEEK_SET) != 0) {
    fmt::print("Unable to seek to relo offsets from: [%s]\r\n", filename);
    return 1;
  }

  std::vector<exe_reloc_table_entry_t> relos;
  relos.resize(hdr.num_relocs);
  if (fread(&relos[0], sizeof(exe_reloc_table_entry_t), hdr.num_relocs, fp) != hdr.num_relocs) {
    fmt::print("Unable to read relo offsets from: [%s]\r\n", filename);
    return 1;
  }

  for (const auto& r : relos) {
    std::cout << "OLD Relo: " << r.segment << ":" << r.offset << std::endl;
  }
  return 11;
}