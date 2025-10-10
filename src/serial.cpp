#include "serial.h"

// Global serial instance
Serial serial(Serial::COM1);

Serial::Serial(Port p) : port(p) {
    init();
}

void Serial::init() {
    // Disable interrupts
    write_port(1, 0x00);
    
    // Enable DLAB (set baud rate divisor)
    write_port(3, 0x80);
    
    // Set divisor to 3 (lo byte) 38400 baud
    write_port(0, 0x03);
    write_port(1, 0x00);
    
    // 8 bits, no parity, one stop bit
    write_port(3, 0x03);
    
    // Enable FIFO, clear them, with 14-byte threshold
    write_port(2, 0xC7);
    
    // IRQs enabled, RTS/DSR set
    write_port(4, 0x0B);
    
    // Set in loopback mode, test the serial chip
    write_port(4, 0x1E);
    
    // Test serial chip (send byte 0xAE and check if serial returns same byte)
    write_port(0, 0xAE);
    
    // Check if serial is faulty (i.e: not same byte as sent)
    if(read_port(0) != 0xAE) {
        return; // Serial is faulty
    }
    
    // If serial is not faulty set it in normal operation mode
    // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
    write_port(4, 0x0F);
}

void Serial::write_port(uint16_t offset, uint8_t value) {
    asm volatile("outb %0, %1" : : "a"(value), "Nd"(port + offset));
}

uint8_t Serial::read_port(uint16_t offset) {
    uint8_t value;
    asm volatile("inb %1, %0" : "=a"(value) : "Nd"(port + offset));
    return value;
}

bool Serial::is_transmit_empty() {
    return read_port(5) & 0x20;
}

void Serial::putchar(char c) {
    // Wait for transmit buffer to be empty
    while (!is_transmit_empty());
    
    // Convert LF to CRLF for proper terminal display
    if (c == '\n') {
        write_port(0, '\r');
        while (!is_transmit_empty());
    }
    
    write_port(0, c);
}

void Serial::puts(const char* str) {
    while (*str) {
        putchar(*str++);
    }
}

void Serial::put_hex(uint32_t value) {
    puts("0x");
    for (int i = 7; i >= 0; i--) {
        uint8_t nibble = (value >> (i * 4)) & 0xF;
        char c = nibble < 10 ? '0' + nibble : 'A' + (nibble - 10);
        putchar(c);
    }
}

void Serial::put_hex64(uint64_t value) {
    puts("0x");
    for (int i = 15; i >= 0; i--) {
        uint8_t nibble = (value >> (i * 4)) & 0xF;
        char c = nibble < 10 ? '0' + nibble : 'A' + (nibble - 10);
        putchar(c);
    }
}
