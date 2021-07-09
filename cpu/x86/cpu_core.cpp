#include "cpu/x86/cpu_core.h"

#include "fmt/format.h"
#include <string>

namespace door86::cpu::x86 {

// AX=0A07  BX=FF00  CX=0000  DX=1818  SP=0080  BP=0000  SI=0000  DI=0000
// DS = 1398 ES = 1398 SS = 1983 CS = 13A8 IP = 000C NV UP EI PL NZ NA PO NC

std::string cpu_core::DebugString() const {
  auto& x = regs.x;
  auto& s = sregs;
  auto& f = flags;
  return fmt::format(
      R"(            AX={:04X}  BX={:04X}  CX={:04X}  DX={:04X}  SP={:04X}  BP={:04X}  SI={:04X}  DI={:04X}
                                          DS={:04X} ES={:04X} SS={:04X} CS={:04X} IP={:04X}) C:{} P:{} Z:{} S:{} O:{})",
      x.ax, x.bx, x.cx, x.dx, x.sp, x.bp, x.si, x.di, s.ds, s.es, s.ss, s.cs, ip,
      flags.cflag() ? 1 : 0, f.pflag() ? 1 : 0, f.zflag() ? 1 : 0, f.sflag() ? 1 : 0,
      f.oflag() ? 1 : 0);
}


} // namespace door86::cpu::x86