#include <cstdio>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>

struct exe_header_t {
  uint16_t signature; /* == 0x5a4D */
  uint16_t bytes_in_last_block;
  uint16_t blocks_in_file;
  uint16_t num_relocs;
  uint16_t header_paragraphs;
  uint16_t min_extra_paragraphs;
  uint16_t max_extra_paragraphs;
  uint16_t ss;
  uint16_t sp;
  uint16_t checksum;
  uint16_t ip;
  uint16_t cs;
  uint16_t reloc_table_offset;
  uint16_t overlay_number;
};

struct exe_reloc_table_entry_t {
  uint16_t offset;
  uint16_t segment;
};

static std::string to_seg_off(uint16_t seg, uint16_t off) {
  std::ostringstream ss;
  ss << std::hex << std::setw(4) << seg << ":" << off << " (" << std::dec << (seg * 16) + off << ")";
  return ss.str();
}

int main(int argc, char** argv) {
  FILE* fp = nullptr;
  if (argc < 2) {
    fprintf(stderr, "Usage: dis <exename>\r\n");
    return 1;
  }
  if (fp = fopen(argv[1], "rb"); !fp) {
    fprintf(stderr, "Unable to open file: [%s]\r\n", argv[1]);
    return 1;
  }

  exe_header_t hdr{};
  if (const auto num_read = fread(&hdr, sizeof(exe_header_t), 1, fp); num_read != 1) {
    fprintf(stderr, "Unable to read header from: [%s]\r\n", argv[1]);
    return 1;
  }
  std::cout << "sizeof(exe_header_t)       " << sizeof(exe_header_t) << std::endl;
  int binary_size = ((hdr.blocks_in_file * 512) + hdr.bytes_in_last_block);
  if (hdr.bytes_in_last_block) {
    binary_size -= 512;
  }
  std::cout << "binary size:               " << binary_size << std::endl;
  std::cout << "num_relocs:                " << hdr.num_relocs << std::endl;
  std::cout << "Magic number :             " << hdr.signature << std::endl;
  std::cout << "Bytes on last page :       " << hdr.bytes_in_last_block << std::endl;
  std::cout << "Pages in file :            " << hdr.blocks_in_file << std::endl;
  std::cout << "Relocations :              " << hdr.num_relocs << std::endl;
  std::cout << "Paragraphs in header :     " << hdr.header_paragraphs << std::endl;
  std::cout << "Header Size :              " << (16 * hdr.header_paragraphs) << std::endl;
  std::cout << "Extra paragraphs needed :  " << hdr.min_extra_paragraphs << std::endl;
  std::cout << "Extra paragraphs wanted :  " << hdr.max_extra_paragraphs << std::endl;
  std::cout << "Initial stack location :   " << to_seg_off(hdr.ss, hdr.sp) << std::endl;
  std::cout << "SP:                        0x" << std::hex << hdr.sp << std::endl;
  std::cout << "Word checksum :            0x" << std::hex << hdr.checksum << std::endl;
  std::cout << "Entry point :              " << hdr.ip << std::endl;
  std::cout << "Relocation table address : 0x" << std::hex << hdr.reloc_table_offset << std::endl;
  std::cout << "Memory needed :            ???" << std::endl;// << std::hex << hdr.
  std::cout << "CS:                        0x" << std::hex << hdr.cs << std::endl;
  std::cout << "Entry point:               " << (hdr.cs * 16) + hdr.ip << std::endl;
  std::cout << "\r\n\n";
  
  
  if (fseek(fp, hdr.reloc_table_offset, SEEK_SET) != 0) {
    fprintf(stderr, "Unable to seek to relo offsets from: [%s]\r\n", argv[1]);
    return 1;
  }

  std::vector<exe_reloc_table_entry_t> relos;
  relos.resize(hdr.num_relocs);
  if (fread(&relos[0], sizeof(exe_reloc_table_entry_t), hdr.num_relocs, fp) != hdr.num_relocs) {
    fprintf(stderr, "Unable to read relo offsets from: [%s]\r\n", argv[1]);
    return 1;
  }

  for (const auto& r : relos) {
    std::cout << "OLD Relo: " << r.segment << ":" << r.offset << std::endl;
  }
  return 11;
}