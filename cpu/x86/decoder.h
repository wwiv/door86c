#ifndef INCLUDED_CPU_X86_DECODER_H
#define INCLUDED_CPU_X86_DECODER_H

#include "cpu/x86/core.h"

#include <string>

namespace door86::cpu::x86 {

constexpr uint32_t op_mask_none = 0x00;
constexpr uint32_t op_mask_imm8 = 0x01;
constexpr uint32_t op_mask_imm16 = 0x02;
constexpr uint32_t op_mask_imm32 = 0x04;
constexpr uint32_t op_mask_ext = 0x08;
constexpr uint32_t op_mask_modrm8 = 0x10;
constexpr uint32_t op_mask_modrm16 = 0x20;
constexpr uint32_t op_mask_modrm32 = 0x40;

constexpr uint32_t op_mask_reg_is_sreg = 0x100;
// this one is not yet implemented
constexpr uint32_t op_mask_notimpl = 0x80000000;

enum class op_enc_t { none, r_rm, rm_r };

struct op_code_data_t {
  uint8_t op;
  uint32_t mask;
  std::string name;
  int bits{16}; // 8, 16, etc/
  op_enc_t op_enc{op_enc_t::rm_r};
};

struct instruction_t {
  uint8_t op;
  reg_mod_rm mdrm;
  uint8_t operand8;
  uint16_t operand16;
  int len{0};
};

reg_mod_rm parse_modrm(uint8_t b);

/** Fetches the next instruction from the bytestream at o */
instruction_t next_instruction(uint8_t* o);

// Op list. Visible for testing.
extern op_code_data_t op_code_data[];

std::string rmreg8_to_string(uint8_t);
std::string rmreg16_to_string(uint8_t);

// Sreg — A segment register.
// The segment register bit assignments are:
// ES = 0, CS = 1, SS = 2, DS = 3, FS = 4, and GS = 5.

std::string rmreg_sreg_to_string(uint8_t);
std::string to_string(const instruction_t&);

} // namespace door86::cpu::x86

#endif // INCLUDED_CPU_X86_DECODER_H