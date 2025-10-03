#ifndef VGA_H
#define VGA_H

#include "types.h"

class VGA {
public:
    enum Color : uint8_t {
        BLACK = 0,
        BLUE = 1,
        GREEN = 2,
        CYAN = 3,
        RED = 4,
        MAGENTA = 5,
        BROWN = 6,
        LIGHT_GREY = 7,
        DARK_GREY = 8,
        LIGHT_BLUE = 9,
        LIGHT_GREEN = 10,
        LIGHT_CYAN = 11,
        LIGHT_RED = 12,
        LIGHT_MAGENTA = 13,
        LIGHT_BROWN = 14,
        WHITE = 15,
    };

    VGA();
    void clear();
    void putchar(char c);
    void puts(const char* str);
    void put_hex(uint64_t value);
    void set_color(Color fg, Color bg);

private:
    static constexpr uint16_t VGA_WIDTH = 80;
    static constexpr uint16_t VGA_HEIGHT = 25;
    static constexpr uintptr_t VGA_MEMORY = 0xB8000;

    uint16_t* buffer;
    uint16_t row;
    uint16_t col;
    uint8_t color;

    void newline();
    void scroll();
    uint16_t make_vga_entry(char c);
};

extern VGA vga;

#endif // VGA_H
