#ifndef INCLUDED_CPU_X86_DECODER_H
#define INCLUDED_CPU_X86_DECODER_H

#include "cpu/x86/core.h"

#include <string>

namespace door86::cpu::x86{

constexpr uint8_t op_mask_none = 0x00;
constexpr uint8_t op_mask_imm8 = 0x01;
constexpr uint8_t op_mask_imm16   = 0x02;
constexpr uint8_t op_mask_imm32   = 0x04;
constexpr uint8_t op_mask_modrm8  = 0x08;
constexpr uint8_t op_mask_modrm16 = 0x10;
constexpr uint8_t op_mask_modrm32 = 0x20;
// The reg field in mod/rm is an extension
constexpr uint8_t op_mask_ext = 0x40;
// this one is not yet implemented
constexpr uint8_t op_mask_notimpl = 0x80;

struct op_code_data_t {
  uint8_t op;
  uint8_t mask;
  std::string name;
};

struct instruction_t {
  uint8_t op;
  uint8_t mdrm;
  uint8_t operand8;
  uint16_t operand16;
};

reg_mod_rm parse_modrm(uint8_t b);

/** Fetches the next instruction from the bytestream at o */
instruction_t next_instruction(uint8_t* o);

// Op list. Visible for testing.
extern op_code_data_t op_code_data[];
}

#endif  // INCLUDED_CPU_X86_DECODER_H