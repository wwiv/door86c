#ifndef INCLUDED_CPU_IO_H
#define INCLUDED_CPU_IO_H

#include <cstdint>
#include <type_traits>

namespace door86::cpu {

/**
 * IO Subsystem.
 */
class IO {
  public: 
    IO() = default;
    ~IO() = default;

    uint8_t inb(uint16_t port);
    uint16_t inw(uint16_t port);

    void outb(uint16_t port, uint8_t value);
    void outw(uint16_t port, uint16_t value);
};
}

#endif