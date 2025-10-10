#include "memory.h"

void *sse_memset(void *dest, int value, size_t count) {
  uint8_t *dest_ptr = static_cast<uint8_t *>(dest);
  uint8_t byte_value = static_cast<uint8_t>(value);

  // Handle small sizes with scalar loop
  if (count < 16) {
    for (size_t i = 0; i < count; i++) {
      dest_ptr[i] = byte_value;
    }
    return dest;
  }

  // Broadcast byte value to 64-bit
  uint64_t value_64 = byte_value;
  value_64 |= value_64 << 8;
  value_64 |= value_64 << 16;
  value_64 |= value_64 << 32;

  // Align destination to 16-byte boundary
  uintptr_t addr = reinterpret_cast<uintptr_t>(dest_ptr);
  size_t misalign = addr & 15;

  if (misalign != 0) {
    size_t align_bytes = 16 - misalign;
    if (align_bytes > count) {
      align_bytes = count;
    }

    for (size_t i = 0; i < align_bytes; i++) {
      dest_ptr[i] = byte_value;
    }

    dest_ptr += align_bytes;
    count -= align_bytes;
  }

  // SSE bulk fill
  if (count >= 16) {
    // Load broadcast value into XMM0
    asm volatile("movq %0, %%xmm0\n"
                 "punpcklqdq %%xmm0, %%xmm0\n" // Duplicate to fill 128 bits
                 :
                 : "r"(value_64)
                 : "xmm0");

    size_t blocks = count / 16;

    for (size_t i = 0; i < blocks; i++) {
      asm volatile("movdqa %%xmm0, (%0)\n" : : "r"(dest_ptr) : "memory");
      dest_ptr += 16;
    }

    count &= 15; // Remainder
  }

  // Handle remainder
  for (size_t i = 0; i < count; i++) {
    dest_ptr[i] = byte_value;
  }

  return dest;
}

void *sse_memset16(void *dest, uint16_t value, size_t count_words) {
  uint16_t *dest_ptr = static_cast<uint16_t *>(dest);

  // Handle small sizes with scalar loop
  if (count_words < 8) { // 8 words = 16 bytes
    for (size_t i = 0; i < count_words; i++) {
      dest_ptr[i] = value;
    }
    return dest;
  }

  // Broadcast 16-bit value to 64-bit pattern (4 words)
  uint64_t pattern = static_cast<uint64_t>(value);
  pattern |= pattern << 16;
  pattern |= pattern << 32;

  // Align destination to 16-byte boundary
  uintptr_t addr = reinterpret_cast<uintptr_t>(dest_ptr);
  size_t misalign_bytes = addr & 15;
  size_t misalign_words = misalign_bytes / 2;

  if (misalign_words != 0) {
    size_t align_words = (16 - misalign_bytes) / 2;
    if (align_words > count_words) {
      align_words = count_words;
    }

    for (size_t i = 0; i < align_words; i++) {
      dest_ptr[i] = value;
    }

    dest_ptr += align_words;
    count_words -= align_words;
  }

  // SSE bulk fill (8 words = 16 bytes per iteration)
  if (count_words >= 8) {
    // Load broadcast value into XMM0 and duplicate
    asm volatile(
        "movq %0, %%xmm0\n"
        "punpcklqdq %%xmm0, %%xmm0\n" // Duplicate to fill 128 bits (8 words)
        :
        : "r"(pattern)
        : "xmm0");

    size_t blocks = count_words / 8;
    uint8_t *byte_dest = reinterpret_cast<uint8_t *>(dest_ptr);
    uintptr_t byte_addr = reinterpret_cast<uintptr_t>(byte_dest);
    bool aligned = ((byte_addr & 15) == 0);

    if (aligned) {
      for (size_t i = 0; i < blocks; i++) {
        asm volatile("movdqa %%xmm0, (%0)\n" : : "r"(byte_dest) : "memory");
        byte_dest += 16;
      }
    } else {
      for (size_t i = 0; i < blocks; i++) {
        asm volatile("movdqu %%xmm0, (%0)\n" : : "r"(byte_dest) : "memory");
        byte_dest += 16;
      }
    }

    dest_ptr = reinterpret_cast<uint16_t *>(byte_dest);
    count_words &= 7; // Remainder (count_words % 8)
  }

  // Handle remainder
  for (size_t i = 0; i < count_words; i++) {
    dest_ptr[i] = value;
  }

  return dest;
}

void *sse_memcpy(void *dest, const void *src, size_t count) {
  uint8_t *dest_ptr = static_cast<uint8_t *>(dest);
  const uint8_t *src_ptr = static_cast<const uint8_t *>(src);

  // Handle small sizes with scalar loop
  if (count < 16) {
    // Check for overlap requiring backward copy
    if (dest_ptr > src_ptr && dest_ptr < src_ptr + count) {
      // Backward copy for small overlapping regions
      for (size_t i = count; i > 0; i--) {
        dest_ptr[i - 1] = src_ptr[i - 1];
      }
    } else {
      // Forward copy for non-overlapping or safe overlap
      for (size_t i = 0; i < count; i++) {
        dest_ptr[i] = src_ptr[i];
      }
    }
    return dest;
  }

  // Check for overlapping regions requiring backward copy
  // If dest > src and dest < src+count, ranges overlap and need backward copy
  if (dest_ptr > src_ptr && dest_ptr < src_ptr + count) {
    // Perform backward copy using SSE

    // Move pointers to end of regions
    dest_ptr += count;
    src_ptr += count;

    // Handle unaligned tail bytes first (backward)
    size_t remainder = count & 15;
    if (remainder > 0) {
      for (size_t i = 0; i < remainder; i++) {
        dest_ptr--;
        src_ptr--;
        *dest_ptr = *src_ptr;
      }
      count -= remainder;
    }

    // SSE bulk copy backward (16-byte blocks)
    if (count >= 16) {
      size_t blocks = count / 16;

      for (size_t i = 0; i < blocks; i++) {
        dest_ptr -= 16;
        src_ptr -= 16;

        // Use unaligned operations for backward copy (simpler and safer)
        asm volatile("movdqu (%1), %%xmm0\n"
                     "movdqu %%xmm0, (%0)\n"
                     :
                     : "r"(dest_ptr), "r"(src_ptr)
                     : "xmm0", "memory");
      }
    }

    return dest;
  }

  // Forward copy path (non-overlapping or safe overlap)

  // Check alignment of both source and destination
  uintptr_t dest_addr = reinterpret_cast<uintptr_t>(dest_ptr);
  uintptr_t src_addr = reinterpret_cast<uintptr_t>(src_ptr);
  size_t dest_misalign = dest_addr & 15;
  size_t src_misalign = src_addr & 15;

  // Align destination if needed
  if (dest_misalign != 0) {
    size_t align_bytes = 16 - dest_misalign;
    if (align_bytes > count) {
      align_bytes = count;
    }

    for (size_t i = 0; i < align_bytes; i++) {
      dest_ptr[i] = src_ptr[i];
    }

    dest_ptr += align_bytes;
    src_ptr += align_bytes;
    count -= align_bytes;
  }

  // SSE bulk copy
  if (count >= 16) {
    size_t blocks = count / 16;
    bool src_aligned = ((reinterpret_cast<uintptr_t>(src_ptr) & 15) == 0);

    if (src_aligned) {
      // Both aligned - use aligned loads and stores
      for (size_t i = 0; i < blocks; i++) {
        asm volatile("movdqa (%1), %%xmm0\n"
                     "movdqa %%xmm0, (%0)\n"
                     :
                     : "r"(dest_ptr), "r"(src_ptr)
                     : "xmm0", "memory");
        dest_ptr += 16;
        src_ptr += 16;
      }
    } else {
      // Source unaligned - use unaligned loads, aligned stores
      for (size_t i = 0; i < blocks; i++) {
        asm volatile("movdqu (%1), %%xmm0\n"
                     "movdqa %%xmm0, (%0)\n"
                     :
                     : "r"(dest_ptr), "r"(src_ptr)
                     : "xmm0", "memory");
        dest_ptr += 16;
        src_ptr += 16;
      }
    }

    count &= 15; // Remainder
  }

  // Handle remainder
  for (size_t i = 0; i < count; i++) {
    dest_ptr[i] = src_ptr[i];
  }

  return dest;
}
