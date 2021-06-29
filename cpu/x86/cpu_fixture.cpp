#include "cpu/x86/cpu_fixture.h"

#include "core/strings.h"
#include <string>
#include <cctype>

namespace door86::cpu::x86 {

static int hex_digit_to_int(char c) {
  if (std::isdigit(c)) {
    return c - '0';
  }
  c = std::toupper(c);
  if (c >= 'A' && c <= 'F') {
    return c - 'A' + 10;
  }
  // assert?
  return 0;
}

void parse_opcodes_from_line(const std::string& s, std::vector<uint8_t>& ops) {
  int current;
  for (auto iter = std::cbegin(s); iter != std::cend(s); iter++) {
    if (!std::isxdigit(*iter)) {
      // !hex digit
      continue;
    }
    current = hex_digit_to_int(*iter++) << 4;
    if (iter == std::cend(s)) {
      // error here, we shouldn't exit here
      break;
    }
    current |= hex_digit_to_int(*iter);
    ops.push_back(current);
  }
}

std::vector<uint8_t> parse_opcodes_from_line(const std::string& s) {
  std::vector<uint8_t> ops;
  parse_opcodes_from_line(s, ops);
  return ops;
}

std::vector<uint8_t> parse_opcodes_from_textdump(const std::string& s) {
  const auto lines = wwiv::strings::SplitString(s, "\r\n", true);
  std::vector<uint8_t> ops;
  ops.reserve(lines.size() * 16);

  for (const auto& l : lines) {
    if (l.empty()) {
      continue;
    }
    if (!std::isdigit(l.front())) {
      continue;
    }
    if (l.size() < 60) {
      continue;
    }
    const auto s = wwiv::strings::StringTrim(l.substr(10, 48));
    parse_opcodes_from_line(s, ops);
  }
  return ops;
}

}
