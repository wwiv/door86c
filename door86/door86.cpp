#include "core/log.h"
#include "core/command_line.h"
#include "core/scope_exit.h"
#include "core/version.h"
#include "cpu/x86/cpu.h"
#include "dos/dos.h"
#include "dos/exe.h"
#include "fmt/format.h"

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <vector>

using namespace wwiv::core;
using namespace door86::cpu::x86;
using namespace door86::dos;


static std::string to_seg_off(uint16_t seg, uint16_t off) {
  return fmt::format("{:04x}:{:04x}", seg, off);
}


int main(int argc, char** argv) {
  LoggerConfig config;
  Logger::Init(argc, argv, config);

  ScopeExit at_exit(Logger::ExitLogger);
  CommandLine cmdline(argc, argv, "");
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
    std::cout << "Usage: door86 [options] <exename>\r\n" << cmdline.GetHelp() << std::endl;
    return 1;
  }
  const auto& filename = cmdline.remaining().front();

  CPU cpu;
  Dos dos;

  int code_offset = 0x100;
  int memory_needed = 0xffff;
  if (is_exe(filename)) {
    auto o = read_exe_header(filename);
    if (!o) {
      std::cout << "Failed to read exe header for file: " << filename;
      return EXIT_FAILURE;
    }
    auto exe = o.value();
    code_offset = exe.header_size();
    memory_needed = exe.memory_needed();
    auto oseg = dos.mem_mgr.allocate(memory_needed);
    if (!oseg) {
      std::cout << "Failed to allocate memory: " << memory_needed;
      return EXIT_FAILURE;
    }
    const auto psp_seg = oseg.value();
    exe.load_image(psp_seg, cpu.memory);
    cpu.core.sregs.ds = psp_seg;
    cpu.core.sregs.es = psp_seg;

    // skip PSP
    const auto seg = psp_seg + 0x10; 
    cpu.core.sregs.cs = exe.hdr.cs + seg;
    cpu.core.sregs.ss = exe.hdr.ss + seg;
    cpu.core.regs.x.sp = exe.hdr.sp + seg;
    cpu.core.ip = exe.hdr.ip;
    // load_image will relocate relos
  } else {
    //
    // COM file
    //
    auto oseg = dos.mem_mgr.allocate(memory_needed);
    if (!oseg) {
      std::cout << "Failed to allocate memory: " << memory_needed;
      return EXIT_FAILURE;
    }
    const auto psp_seg = oseg.value();
    cpu.core.sregs.ds = psp_seg;
    cpu.core.sregs.es = psp_seg;

    const auto seg = psp_seg + 0x100;
    cpu.core.sregs.cs = seg;
    cpu.core.sregs.ss = seg;
    cpu.core.regs.x.sp = seg;
    cpu.core.ip = 0;
  }

  cpu.core.regs.x.ax = 2; // drive C
  const auto start = std::chrono::system_clock::now();
  bool result = cpu.run();
  const auto end = std::chrono::system_clock::now();
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
  fmt::print("Time elapsed: {}ms ({}us)", ms.count(), us.count());

  return result ? EXIT_SUCCESS : EXIT_FAILURE;
}
