#ifndef INCLUDED_DOS_MCB_H
#define INCLUDED_DOS_MCB_H

#include <cstdint>
#include <string>

namespace door86::dos {

/**
Off Size		Description
00  byte	  'M' 4Dh  member of a MCB chain, (not last)
            'Z' 5Ah  indicates last entry in MCB chain
            other values cause "Memory Allocation Failure" on exit
01  word	  PSP segment address of MCB owner (Process Id)
            possible values:
                  0 = free
                  8 = Allocated by DOS before first user pgm loaded
                  other = Process Id/PSP segment address of owner
03  word	  number of paras related to this MCB (excluding MCB)
05  11bytes	reserved
08  8bytes	ASCII program name, NULL terminated if less than max
              length (DOS 4.x+)
10  nbytes	first byte of actual allocated memory block
            - to find the first MCB in the chain, use  INT 21,52
            - DOS 3.1+ the first memory block contains the DOS data segment
ie., installable drivers, buffers, etc
            - DOS 4.x the first memory block is divided into subsegments,
              with their own memory control blocks; offset 0000h is the first
            - the 'M' and 'Z' are said to represent Mark Zbikowski
            - the MCB chain is often referred to as a linked list, but
              technically isn't */
#pragma pack(push, 1)
struct mcb_t {
  char chain;
  // also used as process id
  uint16_t owner_segment;
  uint16_t num_paragraphs;
  uint8_t reserved[11];
  char program_name[8];
};
#pragma pack(pop)

}

#endif
