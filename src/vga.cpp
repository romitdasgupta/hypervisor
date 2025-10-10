#include "vga.h"
#include "memory.h"

VGA vga;

VGA::VGA()
    : buffer(reinterpret_cast<uint16_t *>(VGA_MEMORY)), row(0), col(0),
      color((BLACK << 4) | LIGHT_GREY) {
  clear();
}

void VGA::clear() {
  // Create 16-bit fill value
  uint16_t fill = make_vga_entry(' ');

  // Fill entire buffer using SSE memset16
  // Total: VGA_WIDTH * VGA_HEIGHT = 80 * 25 = 2000 words
  sse_memset16(buffer, fill, VGA_WIDTH * VGA_HEIGHT);

  row = 0;
  col = 0;
}

void VGA::set_color(Color fg, Color bg) { color = (bg << 4) | fg; }

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

void VGA::puts(const char *str) {
  while (*str) {
    putchar(*str++);
  }
}

void VGA::put_hex(uint64_t value) {
  static constexpr char hex_chars[] = "0123456789ABCDEF";
  char buffer[19]; // "0x" + 16 hex digits + null terminator
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
  // Copy rows 1-24 to rows 0-23 using SSE
  // Size: 24 * 80 * 2 = 3840 bytes
  sse_memcpy(buffer, buffer + VGA_WIDTH, 3840);

  // Clear the last row using SSE memset16
  uint16_t fill = make_vga_entry(' ');
  sse_memset16(buffer + (VGA_HEIGHT - 1) * VGA_WIDTH, fill, VGA_WIDTH);
}