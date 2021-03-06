#ifndef INCLUDED_CPU_X86_DECODER_H
#define INCLUDED_CPU_X86_DECODER_H

#include "cpu/memory.h"
#include <optional>
#include <string>
#include <vector>

namespace door86::cpu::x86 {

struct reg_mod_rm {
  // XX000000
  uint8_t mod{0};
  // 00XXX000
  uint8_t reg{0};
  // 00000XXX
  uint8_t rm{0};
};

constexpr uint32_t op_mask_none = 0x00;
// Has IMM or DISP or REL byte (8-bit)
constexpr uint32_t op_mask_imm8 = 0x01;
constexpr uint32_t op_mask_rel8 = 0x01;
// Has IMM or DISP or REL byte (16-bit)
constexpr uint32_t op_mask_imm16 = 0x02;
constexpr uint32_t op_mask_rel16 = 0x02;
// Has IMM or DISP or REL byte (32-bit)
constexpr uint32_t op_mask_imm32 = 0x04;
constexpr uint32_t op_mask_ext = 0x08;
// Has ModRM byte (8-bit)
constexpr uint32_t op_mask_modrm8 = 0x10;
// Has ModRM byte (16-bit)
constexpr uint32_t op_mask_modrm16 = 0x20;
// Has ModRM byte (32-bit)
constexpr uint32_t op_mask_modrm32 = 0x40;
// Any register value refers to segment register not general register
constexpr uint32_t op_mask_reg_is_sreg = 0x100;
// uses a secondary opcode in the register (not used yet)
constexpr uint32_t op_mask_so_opcode = 0x200;
// Has both r and rm encoded in ModRM byte
constexpr uint32_t op_mask_has_r_and_rm = 0x400;
// Use ZF in rep for terminating the loop.
constexpr uint32_t uses_rep_zf = 0x1000;
// Used reg for subcodes
constexpr uint32_t uses_reg_subcode = 0x2000;
// Used reg for encoded register (+r)
constexpr uint32_t uses_encoded_reg = 0x4000;

// TODO(rushfan): Add metadata for (a) is multiplexed byte with reg, is rep_allowed, 

// this one is not yet implemented
constexpr uint32_t op_mask_notimpl = 0x80000000;

enum class op_enc_t { none, rm_r, r_rm, rm_i, r_i };

struct op_code_data_t {
  uint8_t op;
  uint32_t mask;
  std::string name;
  int bits{16}; // 8, 16, etc/
  op_enc_t op_enc{op_enc_t::none};
  uint8_t r_base{0};
};

class instruction_t {
public:
  uint8_t op{0};
  reg_mod_rm mdrm;
  uint8_t disp8{0};
  uint16_t disp16{0};
  uint8_t imm8{0};
  uint16_t imm16{0};
  int len{0};
  op_code_data_t metadata{};

  // methods.
  bool has_modrm() const;
  /** returns the overridden or default segment to use for this instrction */
  segment_t seg_index() const;

  std::string DebugString() const;

  // prefix instructions
  // TODO(rushfan: merge rep and repne and set direction

  std::optional<segment_t> seg_override;
  bool lock{false};
  bool rep{false};
  bool repne{false};

  std::vector<uint8_t> bytes;
};

reg_mod_rm parse_modrm(uint8_t b);

std::string rmreg8_to_string(uint8_t);
std::string rmreg16_to_string(uint8_t);

// Sreg ? A segment register.
// The segment register bit assignments are:
// ES = 0, CS = 1, SS = 2, DS = 3, FS = 4, and GS = 5.

std::string rmreg_sreg_to_string(uint8_t);
std::string to_string(const instruction_t&);

class Decoder {
public:
  Decoder() : Decoder(true) {}
  explicit Decoder(bool save_bytes);
  ~Decoder() = default;

  /** Fetches the next instruction from the bytestream at o */
  instruction_t decode(const uint8_t* o);
  /** Fetches the next instruction from the bytestream at o */
  instruction_t decode(const std::vector<uint8_t> o);

//  std::string to_string(const instruction_t& i);
  const op_code_data_t& op_data(uint8_t opcode) const { return op_data_.at(opcode); }

private:
  // Op list. Visible for testing.
  std::vector<op_code_data_t> op_data_;
  // should the decoder add bytes to the instruction
  bool save_bytes_{true};
};

} // namespace door86::cpu::x86

#endif // INCLUDED_CPU_X86_DECODER_H