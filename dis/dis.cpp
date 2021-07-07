#include <cstdio>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <optional>
#include <vector>
#include "core/log.h"
#include "core/command_line.h"
#include "core/file.h"
#include "core/scope_exit.h"
#include "core/version.h"
#include "cpu/x86/decoder.h"
#include "dos/exe.h"
#include "fmt/format.h"

using namespace wwiv::core;
using namespace door86::cpu::x86;
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


void dump_exe_info(const Exe& exe) {
  std::cout << "binary size:               " << exe.binary_size << std::endl;
  std::cout << "num_relocs:                " << exe.hdr.num_relocs << std::endl;
  fmt::print("EXE Header:                {}\r\n", exe_signature(exe.hdr.signature));
  std::cout << "Bytes on last page :       " << exe.hdr.bytes_in_last_block << std::endl;
  std::cout << "Pages in file :            " << exe.hdr.blocks_in_file << std::endl;
  std::cout << "Relocations :              " << exe.hdr.num_relocs << std::endl;
  std::cout << "Paragraphs in header :     " << exe.hdr.header_paragraphs << std::endl;
  std::cout << "Header Size :              " << (16 * exe.hdr.header_paragraphs) << std::endl;
  std::cout << "Extra paragraphs needed :  " << exe.hdr.min_extra_paragraphs << std::endl;
  std::cout << "Extra paragraphs wanted :  " << exe.hdr.max_extra_paragraphs << std::endl;
  std::cout << "Initial stack location :   " << to_seg_off(exe.hdr.ss, exe.hdr.sp) << std::endl;
  std::cout << "SP:                        0x" << std::hex << exe.hdr.sp << std::endl;
  std::cout << "Word checksum :            0x" << std::hex << exe.hdr.checksum << std::endl;
  std::cout << "Entry point :              " << exe.hdr.ip << std::endl;
  std::cout << "Relocation table address : 0x" << std::hex << exe.hdr.reloc_table_offset
            << std::endl;
  std::cout << "Memory needed :            ???" << std::endl; // << std::hex << info.hdr.
  std::cout << "CS:                        0x" << std::hex << exe.hdr.cs << std::endl;
  std::cout << "Entry point:               " << (exe.hdr.cs * 16) + exe.hdr.ip << std::endl;
  std::cout << "\r\n\n";
  for (const auto& r : exe.relos) {
    std::cout << "OLD Relo: " << r.segment << ":" << r.offset << std::endl;
  }
}

static void disasm_hex(const std::filesystem::path& filepath, int code_offset) { 
  bool done{false};
  FILE* fp = nullptr;
  if (fp = fopen(filepath.string().c_str(), "rb"); !fp) {
    VLOG(1) << "Unable to open file: " << filepath;
    return;
  }

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
    for (size_t i = 0; i < num_read; i++) {
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

static void disasm_code(const std::filesystem::path& filepath, int code_offset) {
  File f(filepath);
  const auto len = f.length();
  if (len <= 0) {
    LOG(WARNING) << "Failed to get size of file: " << filepath;
    return;
  }

  std::vector<uint8_t> ops;
  ops.resize(len);
  if (!f.Open(File::modeBinary | File::modeReadOnly)) {
    LOG(WARNING) << "Failed to open file: " << filepath;
    return;
  }
  const auto num_read = f.Read(&ops[0], len);
  if (num_read != len) {
    LOG(WARNING) << "Failed to read all bytes from file: " << filepath;
    return;
  }
  ops.resize(num_read);

  int pos = code_offset;
  Decoder decoder;
  while (pos < num_read) {
    auto inst = decoder.decode(&ops[pos]);
    fmt::print("{:08x} {}\r\n", pos, inst.DebugString());
    pos += inst.len;
  }
}

static void disasm(const std::filesystem::path& filepath, int code_offset,
                   const std::string& format) {
  if (format == "hex") {
    disasm_hex(filepath, code_offset);
  } else {
    disasm_code(filepath, code_offset);
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
  // Ignore this one. used by logger
  cmdline.add_argument({"v", "verbose log", "0"});
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

  int code_offset = 0;
  if (const auto oinfo = read_exe_header(filename)) {
    // set right code offset to skip header.
    code_offset = oinfo.value().header_size();
    if (cmdline.barg("header")) {
      dump_exe_info(oinfo.value());
    }
  } else {
    std::cout << "Failed to read exe header.";
    return EXIT_FAILURE;
  }

  if (cmdline.barg("disasm")) {
    if (cmdline.barg("header")) {
      fmt::print("\r\nOpcodes: \r\n");
    }
    disasm(filename, code_offset, cmdline.sarg("format"));
  }

  return EXIT_SUCCESS;
}
