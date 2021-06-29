#include <cstdio>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <optional>
#include <vector>
#include "core/log.h"
#include "core/command_line.h"
#include "core/scope_exit.h"
#include "core/version.h"
#include "dos/exe.h"
#include "fmt/format.h"

using namespace wwiv::core;
using namespace door86::dos;


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


void dump_exe_info(const exe_info_t& info) {
  std::cout << "binary size:               " << info.binary_size << std::endl;
  std::cout << "num_relocs:                " << info.hdr.num_relocs << std::endl;
  fmt::print("EXE Header:                {}\r\n", exe_signature(info.hdr.signature));
  std::cout << "Bytes on last page :       " << info.hdr.bytes_in_last_block << std::endl;
  std::cout << "Pages in file :            " << info.hdr.blocks_in_file << std::endl;
  std::cout << "Relocations :              " << info.hdr.num_relocs << std::endl;
  std::cout << "Paragraphs in header :     " << info.hdr.header_paragraphs << std::endl;
  std::cout << "Header Size :              " << (16 * info.hdr.header_paragraphs) << std::endl;
  std::cout << "Extra paragraphs needed :  " << info.hdr.min_extra_paragraphs << std::endl;
  std::cout << "Extra paragraphs wanted :  " << info.hdr.max_extra_paragraphs << std::endl;
  std::cout << "Initial stack location :   " << to_seg_off(info.hdr.ss, info.hdr.sp) << std::endl;
  std::cout << "SP:                        0x" << std::hex << info.hdr.sp << std::endl;
  std::cout << "Word checksum :            0x" << std::hex << info.hdr.checksum << std::endl;
  std::cout << "Entry point :              " << info.hdr.ip << std::endl;
  std::cout << "Relocation table address : 0x" << std::hex << info.hdr.reloc_table_offset
            << std::endl;
  std::cout << "Memory needed :            ???" << std::endl; // << std::hex << info.hdr.
  std::cout << "CS:                        0x" << std::hex << info.hdr.cs << std::endl;
  std::cout << "Entry point:               " << (info.hdr.cs * 16) + info.hdr.ip << std::endl;
  std::cout << "\r\n\n";
  for (const auto& r : info.relos) {
    std::cout << "OLD Relo: " << r.segment << ":" << r.offset << std::endl;
  }
}

void disasm(FILE* fp, int code_offset, const std::string& format) { 
  bool done{false};
  fseek(fp, code_offset, SEEK_SET);
  for (auto row = code_offset; !done; ) {
    char buf[1024];
    memset(buf, 0, sizeof(buf));
    const auto num_read = fread(buf, 1, 1024, fp);
    std::string line1;
    line1.reserve(100);
    std::string line2;
    line2.reserve(100);
    int curline = 0;
    for (auto i = 0; i < num_read; i++) {
      const uint8_t ch = static_cast<uint8_t>(buf[i]);
      line1.append(fmt::format("{:02X} ", ch));
      line2.push_back((ch >= 32 && ch <= 127) ? static_cast<char>(ch) : '.');
      if (++curline == 16) {
        fmt::print("{:08x}  {:48.48}  {}\r\n", row, line1, line2);
        row += 16;
        curline = 0;
        line1.clear();
        line2.clear();
      }
    }
    if (!line1.empty()) {
      fmt::print("{:08x}  {:48.48}  {}\r\n", row, line1, line2);
    }
    if (num_read < 1024) {
      done = true;
    }
  }
}


int main(int argc, char** argv) {
  LoggerConfig config;
  Logger::Init(argc, argv, config);

  ScopeExit at_exit(Logger::ExitLogger);
  CommandLine cmdline(argc, argv, "");
  cmdline.add_argument(BooleanCommandLineArgument{"header", 'H', "Display EXE Header information.", true});
  cmdline.add_argument(BooleanCommandLineArgument{"disasm", 'D', "Display Byte information.", false});
  cmdline.add_argument({"format", 'F', "Format for byte information. (code | hex)", "hex"});
  cmdline.set_no_args_allowed(true);

  if (!cmdline.Parse()) {
    std::cout << cmdline.GetHelp() << std::endl;
    return EXIT_FAILURE;
  }
  if (cmdline.help_requested()) {
    std::cout << cmdline.GetHelp() << std::endl;
    return EXIT_SUCCESS;
  }

  if (cmdline.remaining().empty()) {
    std::cout << "Usage: dis [options] <exename>\r\n" << cmdline.GetHelp() << std::endl;
    return 1;
  }
  const auto& filename = cmdline.remaining().front();
  FILE* fp = nullptr;
  if (fp = fopen(filename.c_str(), "rb"); !fp) {
    fmt::print("Unable to open file: [%s]\r\n", filename);
    return 1;
  }

  char mz[2];
  if (const auto num_read = fread(&mz, 1, 2, fp); num_read != 2) {
    fmt::print("Unable to seek to start from: [%s]\r\n", filename);
    return 1;
  }

  int code_offset = 0x100;
  if (const bool is_exe = (mz[0] == 'M' && mz[1] == 'Z'); is_exe) {
    if (const auto oinfo = read_exe_header(fp, filename)) {
      // set right code offset to skip header.
      code_offset = oinfo.value().hdr.header_paragraphs * 0x10;
      if (cmdline.barg("header")) {
        dump_exe_info(oinfo.value());
      }
    } else {
      std::cout << "Failed to read exe header.";
      return EXIT_FAILURE;
    }
  }

  if (cmdline.barg("disasm")) {
    if (cmdline.barg("header")) {
      fmt::print("\r\nOpcodes: \r\n");
    }
    disasm(fp, code_offset, cmdline.sarg("format"));
  }

  return EXIT_SUCCESS;
}
