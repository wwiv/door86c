#ifndef INCLUDED_CPU_X86_CPU_FIXTURE_H
#define INCLUDED_CPU_X86_CPU_FIXTURE_H

#include <string>
#include <vector>

namespace door86::cpu::x86 {

/**
 * Returns a vector of the opcodes , the opcodes are of the form:
 * FF FF ...
 * FFFF...
 */
std::vector<uint8_t> parse_opcodes_from_line(const std::string& s);

/** 
 * Adds the opcodes to the vector, the opcodes are of the form:
 * FF FF ...
 * FFFF...
 */
void parse_opcodes_from_line(const std::string& s, std::vector<uint8_t>& ops);

// parses opcodes from a dis.exe dump
std::vector<uint8_t> parse_opcodes_from_textdump(const std::string& s);

}

#endif  // INCLUDED_CPU_X86_CPU_FIXTURE_H