#ifndef SERIAL_H
#define SERIAL_H

#include "types.h"

class Serial {
public:
    enum Port {
        COM1 = 0x3F8,
        COM2 = 0x2F8,
        COM3 = 0x3E8,
        COM4 = 0x2E8
    };

    Serial(Port port = COM1);
    void init();
    void putchar(char c);
    void puts(const char* str);
    void put_hex(uint32_t value);
    void put_hex64(uint64_t value);
    bool is_transmit_empty();

private:
    uint16_t port;
    
    void write_port(uint16_t offset, uint8_t value);
    uint8_t read_port(uint16_t offset);
};

// Global serial instance
extern Serial serial;

#endif // SERIAL_H
