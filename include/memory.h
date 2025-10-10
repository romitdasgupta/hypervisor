#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

// SSE-optimized memset: fills memory with byte value
// Uses 16-byte SSE stores for bulk, scalar for remainder
void* sse_memset(void* dest, int value, size_t count);

// SSE-optimized memset for 16-bit values: fills memory with 16-bit word pattern
// Uses 16-byte SSE stores for bulk, scalar for remainder
// count_words specifies the number of 16-bit words to fill
void* sse_memset16(void* dest, uint16_t value, size_t count_words);

// SSE-optimized memcpy: copies memory with memmove semantics
// Handles overlapping regions by detecting overlap and using backward copy when needed
// Uses 16-byte SSE loads/stores for bulk, scalar for remainder
// Safe to use when source and destination regions overlap
void* sse_memcpy(void* dest, const void* src, size_t count);

#endif // MEMORY_H
