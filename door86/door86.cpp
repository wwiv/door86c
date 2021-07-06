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
  Dos dos(&cpu);

  if (!dos.initialize_process(filename)) {
    LOG(WARNING) << "Failed to initialize DOS process";
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
