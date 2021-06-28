#ifndef INCLUDED_CPU_X86_CPU_FIXTURE_H
#define INCLUDED_CPU_X86_CPU_FIXTURE_H

#include <string>
#include <vector>

namespace door86::cpu::x86 {

std::vector<uint8_t> parse_opcodes_from_textdump(const std::string& s);

}

#endif  // INCLUDED_CPU_X86_CPU_FIXTURE_H