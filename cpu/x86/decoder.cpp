#include "cpu/x86/decoder.h"

#include "core/log.h"
#include "fmt/ranges.h"
#include "fmt/format.h"
#include <sstream>
#include <string>
#include <vector>

namespace door86::cpu::x86 {

std::vector<op_code_data_t> create_opcode_metadata() {
  std::vector<op_code_data_t> v{
      // First Octet
      {0x00, op_mask_modrm8, "ADD", 8},
      {0x01, op_mask_modrm16, "ADD"},
      {0x02, op_mask_modrm8, "ADD", 8},
      {0x03, op_mask_modrm16, "ADD"},
      {0x04, op_mask_imm8, "ADD", 8},
      {0x05, op_mask_imm16, "ADD"},
      {0x06, op_mask_none, "PUSH ES"},
      {0x07, op_mask_none, "POP ES"},
      // Next Octet
      {0x08, op_mask_modrm8, "OR", 8},
      {0x09, op_mask_modrm16, "OR"},
      {0x0a, op_mask_modrm8, "OR", 8},
      {0x0b, op_mask_modrm16, "OR"},
      {0x0c, op_mask_imm8, "OR", 8},
      {0x0d, op_mask_imm16, "OR"},
      {0x0e, op_mask_none, "PUSH CS"},
      {0x0f, op_mask_none, "POP CS"},

      {0x10, op_mask_modrm8, "ADC", 8},
      {0x11, op_mask_modrm16, "ADC"},
      {0x12, op_mask_modrm8, "ADC", 8},
      {0x13, op_mask_modrm16, "ADC"},
      {0x14, op_mask_imm8, "ADC", 8},
      {0x15, op_mask_imm16, "ADC"},
      {0x16, op_mask_none, "PUSH SS"},
      {0x17, op_mask_none, "POP SS"},
      {0x18, op_mask_modrm8, "SBB", 8},
      {0x19, op_mask_modrm16, "SBB"},
      {0x1a, op_mask_modrm8, "SBB", 8},
      {0x1b, op_mask_modrm16, "SBB"},
      {0x1c, op_mask_imm8, "SBB", 8},
      {0x1d, op_mask_imm16, "SBB"},
      {0x1e, op_mask_none, "PUSH DS"},
      {0x1f, op_mask_none, "POP DS"},

      {0x20, op_mask_modrm8, "AND", 8},
      {0x21, op_mask_modrm16, "AND"},
      {0x22, op_mask_modrm8, "AND", 8},
      {0x23, op_mask_modrm16, "AND"},
      {0x24, op_mask_imm8, "AND", 8},
      {0x25, op_mask_imm16, "AND"},
      {0x26, op_mask_notimpl, ""},
      {0x27, op_mask_notimpl, ""},
      {0x28, op_mask_modrm8, "SUB", 8},
      {0x29, op_mask_modrm16, "SUB"},
      {0x2a, op_mask_modrm8, "SUB", 8},
      {0x2b, op_mask_modrm16, "SUB"},
      {0x2c, op_mask_imm8, "SUB", 8},
      {0x2d, op_mask_imm16, "SUB"},
      {0x2e, op_mask_notimpl, ""},
      {0x2f, op_mask_notimpl, ""},

      {0x30, op_mask_modrm8, "XOR", 8, op_enc_t::rm_r},
      {0x31, op_mask_modrm16, "XOR", 16, op_enc_t::rm_r},
      {0x32, op_mask_modrm8, "XOR", 8, op_enc_t::r_rm},
      {0x33, op_mask_modrm16, "XOR", 16, op_enc_t::r_rm},
      {0x34, op_mask_imm8, "XOR", 8, op_enc_t::r_i},
      {0x35, op_mask_imm16, "XOR", 16, op_enc_t::r_i},
      {0x36, op_mask_notimpl, ""},
      {0x37, op_mask_notimpl, ""},
      {0x38, op_mask_modrm8, "CMP", 8},
      {0x39, op_mask_modrm16, "CMP"},
      {0x3a, op_mask_modrm8, "CMP", 8},
      {0x3b, op_mask_modrm16, "CMP"},
      {0x3C, op_mask_imm8, "CMP", 8},
      {0x3D, op_mask_imm16, "CMP"},
      {0x3e, op_mask_notimpl, ""},
      {0x3f, op_mask_notimpl, ""},

      {0x40, op_mask_none, "INC"},
      {0x41, op_mask_none, "INC"},
      {0x42, op_mask_none, "INC"},
      {0x43, op_mask_none, "INC"},
      {0x44, op_mask_none, "INC"},
      {0x45, op_mask_none, "INC"},
      {0x46, op_mask_none, "INC"},
      {0x47, op_mask_none, "INC"},
      {0x48, op_mask_none, "DEC"},
      {0x49, op_mask_none, "DEC"},
      {0x4a, op_mask_none, "DEC"},
      {0x4b, op_mask_none, "DEC"},
      {0x4c, op_mask_none, "DEC"},
      {0x4d, op_mask_none, "DEC"},
      {0x4e, op_mask_none, "DEC"},
      {0x4f, op_mask_none, "DEC"},

      {0x50, op_mask_none, "PUSH"},
      {0x51, op_mask_none, "PUSH"},
      {0x52, op_mask_none, "PUSH"},
      {0x53, op_mask_none, "PUSH"},
      {0x54, op_mask_none, "PUSH"},
      {0x55, op_mask_none, "PUSH"},
      {0x56, op_mask_none, "PUSH"},
      {0x57, op_mask_none, "PUSH"},
      {0x58, op_mask_none, "POP"},
      {0x59, op_mask_none, "POP"},
      {0x5a, op_mask_none, "POP"},
      {0x5b, op_mask_none, "POP"},
      {0x5c, op_mask_none, "POP"},
      {0x5d, op_mask_none, "POP"},
      {0x5e, op_mask_none, "POP"},
      {0x5f, op_mask_none, "POP"},

      {0x60, op_mask_none, "PUSHA"},
      {0x61, op_mask_none, "POPA"},
      {0x62, op_mask_notimpl, ""},
      {0x63, op_mask_notimpl, ""},
      {0x64, op_mask_notimpl, ""},
      {0x65, op_mask_notimpl, ""},
      {0x66, op_mask_notimpl, ""},
      {0x67, op_mask_notimpl, ""},
      {0x68, op_mask_imm16, "PUSH"},
      {0x69, op_mask_notimpl, ""},
      {0x6a, op_mask_imm8, "PUSH", 8},
      {0x6b, op_mask_notimpl, ""},
      {0x6c, op_mask_notimpl | uses_rep_zf, ""},
      {0x6d, op_mask_notimpl | uses_rep_zf, ""},
      {0x6e, op_mask_notimpl | uses_rep_zf, ""},
      {0x6f, op_mask_notimpl | uses_rep_zf, ""},

      {0x70, op_mask_rel8, "JO", 8},
      {0x71, op_mask_rel8, "JNO", 8},
      {0x72, op_mask_rel8, "JB/JC/JNAE", 8},
      {0x73, op_mask_rel8, "JNB/JNC/JAE", 8},
      {0x74, op_mask_rel8, "JZ/JE", 8},
      {0x75, op_mask_rel8, "JNZ/JNE", 8},
      {0x76, op_mask_rel8, "JBE/JNA", 8},
      {0x77, op_mask_rel8, "JA/JNBE", 8},
      {0x78, op_mask_rel8, "JS", 8},
      {0x79, op_mask_rel8, "JNS", 8},
      {0x7a, op_mask_rel8, "JP/JPE", 8},
      {0x7b, op_mask_rel8, "JNP/JPO", 8},
      {0x7c, op_mask_rel8, "JL/JNGE", 8},
      {0x7d, op_mask_rel8, "JNL/JGE", 8},
      {0x7e, op_mask_rel8, "JLE/JNG", 8},
      {0x7f, op_mask_rel8, "JNLE/JG", 8},

      {0x80, op_mask_modrm8 | op_mask_imm8 | uses_reg_subcode, "0x80/M", 8},
      {0x81, op_mask_modrm16 | op_mask_imm16 | uses_reg_subcode, "0x81/M", 16},
      {0x82, op_mask_notimpl | op_mask_modrm8 | uses_reg_subcode, "0x82/M", 8},
      // 8 didn't seem right here since we use a modrm16
      {0x83, op_mask_modrm16 | op_mask_imm8 | uses_reg_subcode, "0x83/M", 16},
      {0x84, op_mask_modrm8, "TEST", 8},
      {0x85, op_mask_modrm16, "TEST", 16},
      {0x86, op_mask_modrm8, "XCHG", 8},
      {0x87, op_mask_modrm16, "XCHG", 16},

      // TODO: add the 80s for ADD
      {0x88, op_mask_modrm8, "MOV", 8},
      {0x89, op_mask_modrm16, "MOV"},
      {0x8A, op_mask_modrm8, "MOV", 8},
      {0x8B, op_mask_modrm16, "MOV"},
      {0x8C, op_mask_modrm16 | op_mask_reg_is_sreg, "MOV", 16},
      {0x8D, op_mask_modrm16, "LEA", 16, op_enc_t::r_rm},
      {0x8E, op_mask_modrm16 | op_mask_reg_is_sreg, "MOV", 16, op_enc_t::r_rm},
      {0x8F, op_mask_notimpl | op_mask_modrm16, ""},

      {0x90, op_mask_none, "XCHG"},
      {0x91, op_mask_none, "XCHG"},
      {0x92, op_mask_none, "XCHG"},
      {0x93, op_mask_none, "XCHG"},
      {0x94, op_mask_none, "XCHG"},
      {0x95, op_mask_none, "XCHG"},
      {0x96, op_mask_none, "XCHG"},
      {0x97, op_mask_none, "XCHG"},
      {0x98, op_mask_none, "CBW"},
      {0x99, op_mask_none, "CWD"},
      {0x9a, op_mask_notimpl, ""},
      {0x9b, op_mask_notimpl, ""},
      {0x9c, op_mask_none, "PUSHF"},
      {0x9d, op_mask_none, "POPF"},
      {0x9e, op_mask_notimpl, ""},
      {0x9f, op_mask_notimpl, ""},

      {0xA0, op_mask_imm8, "MOV", 8},
      {0xA1, op_mask_imm16, "MOV", 16},
      {0xA2, op_mask_imm8, "MOV", 8},
      {0xA3, op_mask_imm16, "MOV", 16},
      {0xA4, uses_rep_zf, "MOVS", 8},
      {0xA5, uses_rep_zf, "MOVS", 16},
      {0xA6, uses_rep_zf, "CMPS"},
      {0xA7, uses_rep_zf, "CMPS"},
      {0xA8, op_mask_imm8, "TEST", 8},
      {0xA9, op_mask_imm16, "TEST"},
      {0xAA, uses_rep_zf, "STOS", 8},
      {0xAB, uses_rep_zf, "STOS", 16},
      {0xAC, uses_rep_zf, "LODS", 8},
      {0xAD, uses_rep_zf, "LODS", 16},
      {0xAE, uses_rep_zf, "SCAS", 8},
      {0xAF, uses_rep_zf, "SCAS", 16},

      // MOV with operand encoded in instruction
      {0xB0, op_mask_imm8, "MOV", 8},
      {0xB1, op_mask_imm8, "MOV", 8},
      {0xB2, op_mask_imm8, "MOV", 8},
      {0xB3, op_mask_imm8, "MOV", 8},
      {0xB4, op_mask_imm8, "MOV", 8},
      {0xB5, op_mask_imm8, "MOV", 8},
      {0xB6, op_mask_imm8, "MOV", 8},
      {0xB7, op_mask_imm8, "MOV", 8},
      {0xB8, op_mask_imm16, "MOV"},
      {0xB9, op_mask_imm16, "MOV"},
      {0xBA, op_mask_imm16, "MOV"},
      {0xBB, op_mask_imm16, "MOV"},
      {0xBC, op_mask_imm16, "MOV"},
      {0xBD, op_mask_imm16, "MOV"},
      {0xBE, op_mask_imm16, "MOV"},
      {0xBF, op_mask_imm16, "MOV"},

      {0xC0, op_mask_modrm8 | op_mask_imm8| uses_reg_subcode, "0xC0/M", 8},
      {0xC1, op_mask_modrm16 | uses_reg_subcode | op_mask_imm8, "0xC1/M"},
      {0xC2, op_mask_imm16, "RET"},
      {0xC3, op_mask_none, "RET"},
      {0xC4, op_mask_modrm16, "LES"},
      {0xC5, op_mask_modrm16, "LDS"},
      {0xC6, op_mask_modrm8 | op_mask_imm8, "MOV", 8},
      {0xC7, op_mask_modrm16 | op_mask_imm16, "MOV"},
      {0xC8, op_mask_notimpl, ""},
      {0xC9, op_mask_notimpl, ""},
      {0xCA, op_mask_imm16, "RET"},
      {0xCB, op_mask_none, "RET"},
      // INT3, ib, 4
      {0xCC, op_mask_none, "INT3"},
      {0xCD, op_mask_imm8, "INT", 8},
      {0xCE, op_mask_none, "INTO"},
      {0xCF, op_mask_notimpl, "IRET"},

      {0xD0, op_mask_modrm8 | uses_reg_subcode, "0xD0/M", 8},
      {0xD1, op_mask_modrm16 | uses_reg_subcode, "0xD1/M"},
      {0xD2, op_mask_modrm8 | uses_reg_subcode, "0xD2/M", 8},
      {0xD3, op_mask_modrm16 | uses_reg_subcode, "0xD3/M"},
      {0xD4, op_mask_notimpl, ""},
      {0xD5, op_mask_notimpl, ""},
      {0xD6, op_mask_notimpl, ""},
      {0xD7, op_mask_notimpl, ""},
      // escape bytes (used for FPU)
      {0xD8, op_mask_notimpl | op_mask_imm8, "ESC", 8},
      {0xD9, op_mask_notimpl | op_mask_imm8, "ESC", 8},
      {0xDa, op_mask_notimpl | op_mask_imm8, "ESC", 8},
      {0xDb, op_mask_notimpl | op_mask_imm8, "ESC", 8},
      {0xDc, op_mask_notimpl | op_mask_imm8, "ESC", 8},
      {0xDd, op_mask_notimpl | op_mask_imm8, "ESC", 8},
      {0xDe, op_mask_notimpl | op_mask_imm8, "ESC", 8},
      {0xDf, op_mask_notimpl | op_mask_imm8, "ESC", 8},

      {0xE0, op_mask_rel8, "LOOPNE", 8},
      {0xE1, op_mask_rel8, "LOOPE", 8},
      {0xE2, op_mask_rel8, "LOOP", 8},
      {0xE3, op_mask_rel8, "JCXZ", 8},
      {0xE4, op_mask_imm8, "IN", 8},
      {0xE5, op_mask_imm8, "IN", 8},
      {0xE6, op_mask_imm8, "", 8},
      {0xE7, op_mask_imm8, "", 8},
      {0xE8, op_mask_imm16, "CALL", 16, op_enc_t::none},
      {0xE9, op_mask_imm16, "JMP", 16, op_enc_t::none},
      {0xEa, op_mask_notimpl, ""},
      {0xEB, op_mask_imm8, "JMP", 8},
      {0xEC, op_mask_none, "IN"},
      {0xED, op_mask_none, "IN"},
      {0xEE, op_mask_none, "IN"},
      {0xEF, op_mask_none, "IN"},

      {0xF0, op_mask_notimpl, ""},
      {0xF1, op_mask_notimpl, ""},
      {0xF2, op_mask_notimpl, ""},
      {0xF3, op_mask_notimpl, ""},
      {0xF4, op_mask_notimpl, ""},
      {0xF5, op_mask_none, "CMC"},
      {0xF6, op_mask_modrm8 | uses_reg_subcode, "0xF6/M", 8},
      {0xF7, op_mask_modrm16 | uses_reg_subcode, "0xF7/M"},
      {0xF8, op_mask_none, "CLC"},
      {0xF9, op_mask_none, "STC"},
      {0xFA, op_mask_none, "CLI"},
      {0xFB, op_mask_none, "STL"},
      {0xFC, op_mask_none, "CLD"},
      {0xFD, op_mask_none, "STD"},
      {0xFE, op_mask_modrm8, "INC", 8},
      {0xFF, op_mask_modrm16 | uses_reg_subcode, "0xFF/M"}};

  // Fixup bits
  for (auto& o : v) {
    if ((o.mask & op_mask_imm8) || (o.mask & op_mask_modrm8)) {
      if (o.bits != 8) {
        VLOG(2) << "Let's fix 8-bit for: " << std::hex << static_cast<int>(o.op);
        o.bits = 8;
      }
    } else if (((o.mask & op_mask_imm16) || (o.mask & op_mask_modrm16)) && o.bits != 16) {
      VLOG(2) << "Let's fix 16-bit for: " << std::hex << static_cast<int>(o.op);
      o.bits = 16;
    }
  }
  return v;
}

/*
struct reg_mod_rm {
  // XX000000
  uint8_t mod;
  // 00XXX000
  uint8_t reg;
  // 000000XX
  uint8_t rm;
};
*/

reg_mod_rm parse_modrm(uint8_t b) {
  reg_mod_rm r;
  // mod of 3 means register, 0-2 means some field + disp
  r.mod = (b & 0xc0) >> 6;
  r.reg = (b & 0x38) >> 3;
  r.rm = (b & 0x07);
  return r;
}

inline static bool has_modrm(uint8_t b) {
  static constexpr uint8_t mask_modrm = op_mask_modrm8 | op_mask_modrm16 | op_mask_modrm32;
  return b & mask_modrm;
}

inline static bool has_modrm_register_value(const reg_mod_rm& r) { return r.mod == 0x03; }

inline static bool has_modrm_register_address(const reg_mod_rm& r) {
  return r.mod == 0x00 && r.rm != 0x06;
}

inline static bool has_modrm_operand16(const reg_mod_rm& r) {
  return r.rm == 0x06 || r.mod == 0x02;
}

inline static bool has_modrm_operand8(const reg_mod_rm& r) { return r.rm == 0x01; }

bool instruction_t::has_modrm() const { return door86::cpu::x86::has_modrm(metadata.mask); }

static segment_t default_segment_for_index(uint8_t mod, uint8_t rm) {
  if (rm == 2 || rm == 3 || (rm == 6 && mod != 0)) {
    return segment_t::SS;
  }
  return segment_t::DS;
}

segment_t instruction_t::seg_index() const {
  return seg_override.value_or(default_segment_for_index(mdrm.mod, mdrm.rm));
}

instruction_t Decoder::decode(const std::vector<uint8_t> o) { return decode(o.data()); }

instruction_t Decoder::decode(const uint8_t* o) {
  instruction_t i;
  // TODO(rushfan): add in prefix bytes here.
  i.bytes.push_back(*o);
  const auto first_byte = *o++;
  ++i.len;

  if (first_byte == 0x2e) {
    i.seg_override = segment_t::CS;
    i.bytes.push_back(*o);
    i.op = *o++;
    ++i.len;
  } else if (first_byte == 0x36) {
    i.seg_override = segment_t::SS;
    i.bytes.push_back(*o);
    i.op = *o++;
    ++i.len;
  } else if (first_byte == 0x3E) {
    i.seg_override = segment_t::DS;
    ++i.len;
    i.bytes.push_back(*o);
    i.op = *o++;
  } else if (first_byte == 0x26) {
    i.seg_override = segment_t::ES;
    i.bytes.push_back(*o);
    i.op = *o++;
    ++i.len;
  } else if (first_byte == 0xf0) {
    i.lock = true;
    i.bytes.push_back(*o);
    i.op = *o++;
    ++i.len;
  } else if (first_byte == 0xf2) {
    i.repne = true;
    i.bytes.push_back(*o);
    i.op = *o++;
    ++i.len;
  } else if (first_byte == 0xf3) {
    i.rep = true;
    i.bytes.push_back(*o);
    i.op = *o++;
    ++i.len;
  } else {
    i.op = first_byte;
  }

  i.metadata = op_data_[i.op];
  if (has_modrm(i.metadata.mask)) {
    ++i.len;
    i.bytes.push_back(*o);
    i.mdrm = parse_modrm(*o++);
    if ((i.mdrm.rm == 0x06 && i.mdrm.mod == 0) || i.mdrm.mod == 0x02) {
      i.len += 2;
      // disp16
      i.bytes.push_back(*o);
      auto lsb = *o++;
      i.bytes.push_back(*o);
      auto msb = *o++;
      i.disp16 = (msb << 8) | lsb;
    } else if (i.mdrm.mod == 0x01) {
      i.len++;
      i.bytes.push_back(*o);
      i.disp8 = *o++;
    }
  }
  // Let the metadata immediate bytes replace any that came from
  // modrm bytes
  if (i.metadata.mask & op_mask_imm8) {
    i.len++;
    i.bytes.push_back(*o);
    i.imm8 = *o++;
  } else if (i.metadata.mask & op_mask_imm16) {
    i.len += 2;
    i.bytes.push_back(*o);
    auto lsb = *o++;
    i.bytes.push_back(*o);
    auto msb = *o++;
    i.imm16 = (msb << 8) | lsb;
  }
  return i;
}

std::string rmreg8_to_string(uint8_t reg) {
  switch (reg) {
  case 0: return "AL";
  case 1: return "CL";
  case 2: return "DL";
  case 3: return "BL";
  case 4: return "AH";
  case 5: return "CH";
  case 6: return "DH";
  case 7: return "BH";
  }
  return "??";
}

std::string rmreg16_to_string(uint8_t reg) {
  switch (reg) {
  case 0: return "AX";
  case 1: return "CX";
  case 2: return "DX";
  case 3: return "BX";
  case 4: return "SP";
  case 5: return "BP";
  case 6: return "SI";
  case 7: return "DI";
  }
  return "??";
}

// Sreg � A segment register.
// The segment register bit assignments are:
// ES = 0, CS = 1, SS = 2, DS = 3, FS = 4, and GS = 5.
std::string rmreg_sreg_to_string(uint8_t reg) {
  switch (reg) {
  case 0: return "ES";
  case 1: return "CS";
  case 2: return "SS";
  case 3: return "DS";
  case 4: return "FS";
  case 5: return "GS";
  default: return "??";
  }
}

static char* ea012[] = {"[BX + SI]", "[BX + DI]", "[BP + SI]", "[BP + DI]",
                        "[SI]",      "[DI]",      "[BP]",      "[BX]"};

static char* mod_disp[] = {"", "disp8", "disp16"};

std::string rm_to_string(const reg_mod_rm& rm, int bits, uint8_t operand8, uint16_t operand16) {

  if (rm.mod == 0 && rm.rm == 0x06) {
    return fmt::format("{}", operand16);
  }
  if (rm.mod == 0) {
    return ea012[rm.rm];
  }
  if (rm.mod < 3) {
    return fmt::format("{}+{} ({})", ea012[rm.rm], operand16, mod_disp[rm.mod]);
  }
  if (rm.mod == 3 && bits == 8) {
    return rmreg8_to_string(rm.reg);
  }
  if (rm.mod == 3 && bits == 16) {
    return rmreg16_to_string(rm.reg);
  }
  return {};
}

/*
 std::string Decoder::to_string(const instruction_t& i) {
  const auto& op = op_data_[i.op];
  std::ostringstream ss;
  ss << (op.name.empty() ? "???" : op.name) << " ";
  if (has_modrm(op.mask)) {
    std::string operand;
    std::string rm_reg;
    if (op.mask & op_mask_reg_is_sreg) {
      rm_reg = rmreg_sreg_to_string(i.mdrm.reg);
    } else {
      if (op.bits == 8) {
        rm_reg = rmreg8_to_string(i.mdrm.reg);
      } else {
        rm_reg = rmreg16_to_string(i.mdrm.reg);
      }
    }
    if (has_modrm_operand8(i.mdrm)) {
      operand = rm_to_string(i.mdrm, 8, i.operand8, 0);
    } else if (has_modrm_operand16(i.mdrm)) {
      operand = rm_to_string(i.mdrm, 16, 0, i.operand16);
    } else if (has_modrm_register_value(i.mdrm)) {
      operand = rmreg16_to_string(i.mdrm.rm);
    }
    if (op.op_enc == op_enc_t::r_rm) {
      ss << rm_reg << ", " << operand;
    } else if (op.op_enc == op_enc_t::rm_r) {
      ss << operand << ", " << rm_reg;
    }
    ss << " ";
  }
  if (op.mask & op_mask_imm8) {
    ss << fmt::format("{:02x} ", i.operand8);
  } else if (op.mask & op_mask_imm16) {
    ss << fmt::format("{:02x} ", i.operand16);
  }
  return ss.str();
}
*/

Decoder::Decoder(bool save_bytes) : op_data_(create_opcode_metadata()), save_bytes_(save_bytes) {}

std::string instruction_t::DebugString() const {
  static std::vector<char*> reg16 = {"AX", "CX", "DX", "BX", "SP", "BP", "SI", "DI"};
  static std::vector<char*> reg8 = {"AL", "CL", "DL", "BL", "AH", "CH", "DH", "BH"};

  std::string rs;
  if (rep) {
    rs = "[REP]";
  } else if (repne) {
    rs = "[REPNE]";
  }
  if (seg_override.has_value()) {
    rs += fmt::format("[SO:{}]", segment_names.at(static_cast<int>(seg_index())));
  }
  std::string r;
  std::string disp;
  std::string imm;
  auto name = metadata.name;
  if (metadata.mask & uses_reg_subcode) {
    name.push_back('/');
    name.push_back('0' + mdrm.reg);
  }
  std::string bs;
  if (!bytes.empty()) {
    bs = fmt::format("{:02X}", fmt::join(bytes, ""));
  }
  if (has_modrm()) {
    if (has_modrm_register_value(mdrm)) {
      r = metadata.bits == 8 ? fmt::format(" r:{}", reg8[mdrm.reg])
                             : fmt::format(" r:{}", reg16[mdrm.reg]);
      if (metadata.mask & op_mask_reg_is_sreg) {
        r = fmt::format(" r:{}", segment_names.at(mdrm.reg));
      }
    } else if (metadata.mask & op_mask_reg_is_sreg) {
      r = fmt::format(" r:{}", rmreg_sreg_to_string(mdrm.reg));
    } else {
      if (metadata.bits == 8) {
        r = fmt::format(" r:{}", rmreg8_to_string(mdrm.reg));
      } else {
        r = fmt::format(" r:{}", rmreg16_to_string(mdrm.reg));
      }
    }
    if ((mdrm.rm == 0x06 && mdrm.mod == 0) || mdrm.mod == 0x02) {
      disp = fmt::format(" disp:{:04X}, ", disp16);
    } else if (mdrm.mod == 0x01) {
      disp = fmt::format(" disp:{:02X}, ", disp8);
    }
  }
  if (metadata.mask & op_mask_imm8) {
    imm = fmt::format(" imm:{:02X}, ", imm8);
  } else if (metadata.mask & op_mask_imm16) {
    imm = fmt::format(" imm:{:04X}, ", imm16);
  }
  return fmt::format("{:10} [len:{}]{} 0x{:02X}/{}{}{}", bs, len, rs, op, name, r, imm);
}

} // namespace door86::cpu::x86