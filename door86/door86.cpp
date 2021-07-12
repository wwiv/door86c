#include "bios/bios.h"
#include "core/log.h"
#include "core/command_line.h"
#include "core/net.h"
#include "core/scope_exit.h"
#include "core/version.h"
#include "cpu/x86/cpu.h"
#include "dbg/debugger.h"
#include "dbg/lame_debugger.h"
#include "dos/dos.h"
#include "dos/exe.h"
#include "fmt/format.h"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <thread>
#include <vector>

using namespace wwiv::core;
using namespace door86::cpu::x86;


static std::string to_seg_off(uint16_t seg, uint16_t off) {
  return fmt::format("{:04x}:{:04x}", seg, off);
}

static void HandleDebuggerConnection(door86::dbg::Debugger* di, SOCKET sock) {
  door86::dbg::LameDebugger ld(di, sock);
  ld.Run();
}

std::atomic<bool> need_to_exit;

static void StartDebugger(door86::dbg::Debugger* debugger) {
  auto debugger_fn = [&](accepted_socket_t r) {
    std::thread client(HandleDebuggerConnection, debugger, r.client_socket);
    client.detach();
  };
  SocketSet sockets(10);
  sockets.add(2112, debugger_fn, "LAME_DEBUGGER");
  sockets.Run(need_to_exit);
}

int main(int argc, char** argv) {
  LoggerConfig config;
  Logger::Init(argc, argv, config);

  ScopeExit at_exit(Logger::ExitLogger);
  CommandLine cmdline(argc, argv, "");
  // Ignore this one. used by logger
  cmdline.add_argument({"v", "verbose log", "0"});
  cmdline.add_argument(
      BooleanCommandLineArgument{"debugger", 'D', "Enable lame debugger.", true});
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
  door86::bios::Bios bios(&cpu);
  door86::dos::Dos dos(&cpu);
  door86::dbg::Debugger debugger(&cpu);

  if (!dos.initialize_process(filename)) {
    LOG(ERROR) << "Failed to initialize DOS process";
    return EXIT_FAILURE;
  }

  if (cmdline.barg("debugger")) {
    [[maybe_unused]] static bool initialized = wwiv::core::InitializeSockets();
    std::thread client(StartDebugger, &debugger);
    client.detach();

  }
  cpu.core.regs.x.ax = 2; // drive C
  const auto start = std::chrono::system_clock::now();
  bool result = cpu.run();
  const auto end = std::chrono::system_clock::now();

  need_to_exit.store(true);

  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
  fmt::print("Time elapsed: {}ms ({}us)", ms.count(), us.count());

  return result ? EXIT_SUCCESS : EXIT_FAILURE;
}
