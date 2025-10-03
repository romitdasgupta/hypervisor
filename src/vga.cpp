#include "vga.h"

VGA vga;

VGA::VGA() 
    : buffer(reinterpret_cast<uint16_t*>(VGA_MEMORY)),
      row(0),
      col(0),
      color((BLACK << 4) | LIGHT_GREY) {
    clear();
}

void VGA::clear() {
    for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        buffer[i] = make_vga_entry(' ');
    }
    row = 0;
    col = 0;
}

void VGA::set_color(Color fg, Color bg) {
    color = (bg << 4) | fg;
}

uint16_t VGA::make_vga_entry(char c) {
    return static_cast<uint16_t>(c) | (static_cast<uint16_t>(color) << 8);
}

void VGA::putchar(char c) {
    if (c == '\n') {
        newline();
        return;
    }

    buffer[row * VGA_WIDTH + col] = make_vga_entry(c);
    
    if (++col >= VGA_WIDTH) {
        newline();
    }
}

void VGA::puts(const char* str) {
    while (*str) {
        putchar(*str++);
    }
}

void VGA::put_hex(uint64_t value) {
    const char hex_chars[] = "0123456789ABCDEF";
    char buffer[18];
    buffer[0] = '0';
    buffer[1] = 'x';
    
    for (int i = 15; i >= 0; i--) {
        buffer[2 + (15 - i)] = hex_chars[(value >> (i * 4)) & 0xF];
    }
    buffer[18] = '\0';
    
    puts(buffer);
}

void VGA::newline() {
    col = 0;
    if (++row >= VGA_HEIGHT) {
        scroll();
        row = VGA_HEIGHT - 1;
    }
}

void VGA::scroll() {
    // Move all rows up by one
    for (size_t r = 0; r < VGA_HEIGHT - 1; r++) {
        for (size_t c = 0; c < VGA_WIDTH; c++) {
            buffer[r * VGA_WIDTH + c] = buffer[(r + 1) * VGA_WIDTH + c];
        }
    }
    
    // Clear the last row
    for (size_t c = 0; c < VGA_WIDTH; c++) {
        buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + c] = make_vga_entry(' ');
    }
}