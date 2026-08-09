# String Shift Algorithms

A comprehensive collection of **15 different algorithms** to shift (rotate) a string by `i` characters to the left.

## Overview

String shifting (or rotation) is a common operation where characters from the beginning of a string are moved to the end. For example, shifting "ABCDEFGH" by 3 positions results in "DEFGHABC".

## Usage

```python
from string_shift import *

# Example
original = "ABCDEFGH"
shift_by = 3

result = shift_using_slicing(original, shift_by)
print(result)  # Output: DEFGHABC
```

## Algorithms Implemented

### 1. **String Slicing** (`shift_using_slicing`)
- **Time Complexity:** O(n)
- **Space Complexity:** O(n)
- **Description:** Uses Python's string slicing to extract and concatenate substrings.
- **Best For:** Simple, readable Python code

```python
return s[i:] + s[:i]
```

### 2. **Collections Deque** (`shift_using_deque`)
- **Time Complexity:** O(n)
- **Space Complexity:** O(n)
- **Description:** Uses a double-ended queue which supports efficient rotation.
- **Best For:** When you need built-in rotation functionality

### 3. **Reversal Algorithm** (`shift_using_reversal`)
- **Time Complexity:** O(n)
- **Space Complexity:** O(n)
- **Description:** Three-step reversal process
  1. Reverse first i characters
  2. Reverse remaining n-i characters
  3. Reverse entire string
- **Best For:** In-place operations (in languages with mutable strings)

### 4. **Juggling Algorithm** (`shift_using_juggling`)
- **Time Complexity:** O(n)
- **Space Complexity:** O(n)
- **Description:** Moves elements in cycles based on GCD of n and i.
- **Best For:** Minimal element movements, efficient for large arrays

### 5. **Double String Concatenation** (`shift_using_concatenation`)
- **Time Complexity:** O(n)
- **Space Complexity:** O(n)
- **Description:** Creates a doubled string and extracts a substring of length n.
- **Best For:** Quick implementation, easy to understand

```python
doubled = s + s
return doubled[i:i + n]
```

### 6. **List Comprehension** (`shift_using_list_comprehension`)
- **Time Complexity:** O(n)
- **Space Complexity:** O(n)
- **Description:** Uses list comprehension to map each position to its shifted position.
- **Best For:** Pythonic, functional programming style

### 7. **Iterator-based** (`shift_using_iterator`)
- **Time Complexity:** O(n)
- **Space Complexity:** O(n)
- **Description:** Uses itertools.chain to concatenate two iterators.
- **Best For:** Memory-efficient streaming

### 8. **Array Copy** (`shift_using_array_copy`)
- **Time Complexity:** O(n)
- **Space Complexity:** O(n)
- **Description:** Creates a new array and copies elements to their shifted positions.
- **Best For:** Clear, explicit positioning logic

### 9. **Recursive** (`shift_using_recursive`)
- **Time Complexity:** O(n)
- **Space Complexity:** O(n) - recursion stack
- **Description:** Recursively shifts one character at a time.
- **Best For:** Functional programming, educational purposes

### 10. **Generator Expression** (`shift_using_generator`)
- **Time Complexity:** O(n)
- **Space Complexity:** O(n)
- **Description:** Uses a generator expression for memory-efficient iteration.
- **Best For:** Lazy evaluation, memory efficiency

### 11. **Block Swap Algorithm** (`shift_using_block_swap`)
- **Time Complexity:** O(n)
- **Space Complexity:** O(n)
- **Description:** Divides the string into blocks and swaps them recursively/iteratively.
- **Best For:** Efficient swapping, minimizing data movement

### 12. **Stack-based** (`shift_using_stack`)
- **Time Complexity:** O(n)
- **Space Complexity:** O(n)
- **Description:** Uses two stacks to rearrange the string.
- **Best For:** Demonstrating stack data structure usage

### 13. **Queue-based** (`shift_using_queue`)
- **Time Complexity:** O(n)
- **Space Complexity:** O(n)
- **Description:** Simulates rotation using queue operations.
- **Best For:** Demonstrating queue data structure usage

### 14. **Index Calculation** (`shift_using_bitwise`)
- **Time Complexity:** O(n)
- **Space Complexity:** O(n)
- **Description:** Manually calculates new positions using index arithmetic.
- **Best For:** Low-level implementation, understanding modulo operations

### 15. **Map Function** (`shift_using_map`)
- **Time Complexity:** O(n)
- **Space Complexity:** O(n)
- **Description:** Uses the map function to apply transformation.
- **Best For:** Functional programming style

## Running the Code

```bash
python3 string_shift.py
```

This will:
1. Demonstrate all 15 algorithms with the same input
2. Run comprehensive tests on various edge cases
3. Verify all algorithms produce identical results

## Features

- ✅ Handles edge cases (empty strings, single character, shift > length)
- ✅ Supports negative shifts (right rotation)
- ✅ All algorithms handle modulo automatically
- ✅ Comprehensive test suite included
- ✅ Well-documented with docstrings

## Test Cases

The implementation includes tests for:
- Normal rotation
- Zero rotation
- Rotation greater than string length
- Empty strings
- Single character strings
- Negative rotation

## Performance Comparison

All algorithms have **O(n) time complexity** and **O(n) space complexity** in Python due to string immutability. In languages with mutable strings (like C++), some algorithms (Reversal, Juggling, Block Swap) can achieve **O(1) space complexity**.

### Recommended Algorithms

- **For Python:** `shift_using_slicing` or `shift_using_concatenation` - simplest and most readable
- **For interviews:** `shift_using_reversal` or `shift_using_juggling` - demonstrates algorithmic thinking
- **For production:** `shift_using_deque` - uses built-in, well-tested library

## Educational Value

Each algorithm demonstrates different programming concepts:
- **Data Structures:** Stack, Queue, Deque
- **Techniques:** Slicing, Recursion, Iteration
- **Algorithms:** GCD, Block Swapping, Reversal
- **Python Features:** List comprehension, Generators, Map, Itertools

## License

This code is provided for educational purposes.
