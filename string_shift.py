"""
String Shift Algorithms
=======================
Multiple implementations to shift (rotate) a string by i characters.
Each algorithm demonstrates a different approach to solve the same problem.
"""


def shift_using_slicing(s, i):
    """
    Algorithm 1: String Slicing
    Time Complexity: O(n)
    Space Complexity: O(n)
    
    Uses Python's string slicing to extract and concatenate substrings.
    """
    if not s:
        return s
    
    n = len(s)
    i = i % n  # Handle cases where i > n
    
    return s[i:] + s[:i]


def shift_using_deque(s, i):
    """
    Algorithm 2: Using Collections Deque
    Time Complexity: O(n)
    Space Complexity: O(n)
    
    Uses a double-ended queue which supports efficient rotation.
    """
    from collections import deque
    
    if not s:
        return s
    
    n = len(s)
    i = i % n
    
    dq = deque(s)
    dq.rotate(-i)  # Negative rotation for left shift
    
    return ''.join(dq)


def shift_using_reversal(s, i):
    """
    Algorithm 3: Reversal Algorithm
    Time Complexity: O(n)
    Space Complexity: O(n) - due to string immutability in Python
    
    Three-step reversal process:
    1. Reverse first i characters
    2. Reverse remaining n-i characters
    3. Reverse entire string
    """
    def reverse_string(arr, start, end):
        while start < end:
            arr[start], arr[end] = arr[end], arr[start]
            start += 1
            end -= 1
    
    if not s:
        return s
    
    n = len(s)
    i = i % n
    
    # Convert to list for in-place operations
    arr = list(s)
    
    # Reverse first i characters
    reverse_string(arr, 0, i - 1)
    
    # Reverse remaining characters
    reverse_string(arr, i, n - 1)
    
    # Reverse entire array
    reverse_string(arr, 0, n - 1)
    
    return ''.join(arr)


def shift_using_juggling(s, i):
    """
    Algorithm 4: Juggling Algorithm
    Time Complexity: O(n)
    Space Complexity: O(n) - due to string immutability in Python
    
    Moves elements in cycles based on GCD of n and i.
    """
    from math import gcd
    
    if not s:
        return s
    
    n = len(s)
    i = i % n
    
    arr = list(s)
    cycles = gcd(n, i)
    
    for cycle_start in range(cycles):
        temp = arr[cycle_start]
        j = cycle_start
        
        while True:
            k = (j + i) % n
            if k == cycle_start:
                break
            arr[j] = arr[k]
            j = k
        
        arr[j] = temp
    
    return ''.join(arr)


def shift_using_concatenation(s, i):
    """
    Algorithm 5: Double String Concatenation
    Time Complexity: O(n)
    Space Complexity: O(n)
    
    Creates a doubled string and extracts a substring of length n.
    """
    if not s:
        return s
    
    n = len(s)
    i = i % n
    
    doubled = s + s
    return doubled[i:i + n]


def shift_using_list_comprehension(s, i):
    """
    Algorithm 6: List Comprehension with Modulo
    Time Complexity: O(n)
    Space Complexity: O(n)
    
    Uses list comprehension to map each position to its shifted position.
    """
    if not s:
        return s
    
    n = len(s)
    i = i % n
    
    return ''.join([s[(j + i) % n] for j in range(n)])


def shift_using_iterator(s, i):
    """
    Algorithm 7: Iterator-based Approach
    Time Complexity: O(n)
    Space Complexity: O(n)
    
    Uses itertools.chain to concatenate two iterators.
    """
    from itertools import chain, islice
    
    if not s:
        return s
    
    n = len(s)
    i = i % n
    
    return ''.join(chain(islice(s, i, n), islice(s, 0, i)))


def shift_using_array_copy(s, i):
    """
    Algorithm 8: Array Copying
    Time Complexity: O(n)
    Space Complexity: O(n)
    
    Creates a new array and copies elements to their shifted positions.
    """
    if not s:
        return s
    
    n = len(s)
    i = i % n
    
    result = [''] * n
    
    for j in range(n):
        result[j] = s[(j + i) % n]
    
    return ''.join(result)


def shift_using_recursive(s, i):
    """
    Algorithm 9: Recursive Approach
    Time Complexity: O(n)
    Space Complexity: O(n) - due to recursion stack
    
    Recursively shifts one character at a time.
    """
    if not s or i == 0:
        return s
    
    n = len(s)
    i = i % n
    
    if i == 0:
        return s
    
    # Shift by 1 and recurse
    return shift_using_recursive(s[1:] + s[0], i - 1)


def shift_using_generator(s, i):
    """
    Algorithm 10: Generator Expression
    Time Complexity: O(n)
    Space Complexity: O(n)
    
    Uses a generator expression for memory-efficient iteration.
    """
    if not s:
        return s
    
    n = len(s)
    i = i % n
    
    return ''.join(s[(j + i) % n] for j in range(n))


def shift_using_block_swap(s, i):
    """
    Algorithm 11: Block Swap Algorithm
    Time Complexity: O(n)
    Space Complexity: O(n) - due to string immutability
    
    Divides the string into blocks and swaps them recursively/iteratively.
    """
    def block_swap(arr, start, d, n_elements):
        """Swap blocks of size d and n_elements - d"""
        if d == 0 or d == n_elements:
            return
        
        if d == n_elements - d:
            # Both blocks are equal size
            for j in range(d):
                arr[start + j], arr[start + d + j] = arr[start + d + j], arr[start + j]
            return
        
        if d < n_elements - d:
            # Left block is smaller
            for j in range(d):
                arr[start + j], arr[start + n_elements - d + j] = arr[start + n_elements - d + j], arr[start + j]
            block_swap(arr, start, d, n_elements - d)
        else:
            # Right block is smaller
            for j in range(n_elements - d):
                arr[start + j], arr[start + d + j] = arr[start + d + j], arr[start + j]
            block_swap(arr, start + n_elements - d, 2 * d - n_elements, d)
    
    if not s:
        return s
    
    n = len(s)
    i = i % n
    
    if i == 0:
        return s
    
    arr = list(s)
    block_swap(arr, 0, i, n)
    
    return ''.join(arr)


def shift_using_stack(s, i):
    """
    Algorithm 12: Using Stack (List as Stack)
    Time Complexity: O(n)
    Space Complexity: O(n)
    
    Uses two stacks to rearrange the string.
    """
    if not s:
        return s
    
    n = len(s)
    i = i % n
    
    stack1 = list(s[:i])
    stack2 = list(s[i:])
    
    result = stack2 + stack1
    
    return ''.join(result)


def shift_using_queue(s, i):
    """
    Algorithm 13: Using Queue
    Time Complexity: O(n)
    Space Complexity: O(n)
    
    Simulates rotation using queue operations.
    """
    from collections import deque
    
    if not s:
        return s
    
    n = len(s)
    i = i % n
    
    queue = deque(s)
    
    # Dequeue i elements and enqueue them at the end
    for _ in range(i):
        queue.append(queue.popleft())
    
    return ''.join(queue)


def shift_using_bitwise(s, i):
    """
    Algorithm 14: Character-by-Character with Index Calculation
    Time Complexity: O(n)
    Space Complexity: O(n)
    
    Manually calculates new positions using index arithmetic.
    """
    if not s:
        return s
    
    n = len(s)
    i = i % n
    
    result = []
    
    for idx in range(n):
        new_idx = (idx + i) % n
        result.append(s[new_idx])
    
    return ''.join(result)


def shift_using_map(s, i):
    """
    Algorithm 15: Using Map Function
    Time Complexity: O(n)
    Space Complexity: O(n)
    
    Uses the map function to apply transformation.
    """
    if not s:
        return s
    
    n = len(s)
    i = i % n
    
    return ''.join(map(lambda idx: s[(idx + i) % n], range(n)))


# Testing and demonstration
def demonstrate_all_algorithms():
    """
    Demonstrates all algorithms with the same input.
    """
    test_string = "ABCDEFGH"
    shift_amount = 3
    
    algorithms = [
        ("Slicing", shift_using_slicing),
        ("Deque", shift_using_deque),
        ("Reversal", shift_using_reversal),
        ("Juggling", shift_using_juggling),
        ("Concatenation", shift_using_concatenation),
        ("List Comprehension", shift_using_list_comprehension),
        ("Iterator", shift_using_iterator),
        ("Array Copy", shift_using_array_copy),
        ("Recursive", shift_using_recursive),
        ("Generator", shift_using_generator),
        ("Block Swap", shift_using_block_swap),
        ("Stack", shift_using_stack),
        ("Queue", shift_using_queue),
        ("Bitwise/Index", shift_using_bitwise),
        ("Map", shift_using_map),
    ]
    
    print(f"Original String: {test_string}")
    print(f"Shift Amount: {shift_amount}")
    print("-" * 60)
    
    for name, func in algorithms:
        result = func(test_string, shift_amount)
        print(f"{name:20s}: {result}")
    
    # Verify all results are the same
    results = [func(test_string, shift_amount) for _, func in algorithms]
    if len(set(results)) == 1:
        print("\n✓ All algorithms produce the same result!")
    else:
        print("\n✗ Warning: Algorithms produced different results!")


def run_comprehensive_tests():
    """
    Runs comprehensive tests on all algorithms.
    """
    test_cases = [
        ("ABCDEFGH", 3, "DEFGHABC"),
        ("Hello", 2, "lloHe"),
        ("Python", 0, "Python"),
        ("Test", 7, "tTes"),  # i > len(s) - 7 % 4 = 3
        ("AB", 1, "BA"),
        ("", 5, ""),
        ("X", 100, "X"),
        ("12345", -1, "51234"),  # Negative shift
    ]
    
    algorithms = [
        shift_using_slicing,
        shift_using_deque,
        shift_using_reversal,
        shift_using_juggling,
        shift_using_concatenation,
        shift_using_list_comprehension,
        shift_using_iterator,
        shift_using_array_copy,
        shift_using_recursive,
        shift_using_generator,
        shift_using_block_swap,
        shift_using_stack,
        shift_using_queue,
        shift_using_bitwise,
        shift_using_map,
    ]
    
    print("Running Comprehensive Tests...")
    print("=" * 60)
    
    all_passed = True
    
    for s, i, expected in test_cases:
        print(f"\nTest: shift('{s}', {i}) => Expected: '{expected}'")
        
        for func in algorithms:
            result = func(s, i)
            status = "✓" if result == expected else "✗"
            
            if result != expected:
                print(f"  {status} {func.__name__}: '{result}' (FAILED)")
                all_passed = False
            else:
                print(f"  {status} {func.__name__}: '{result}'")
    
    print("\n" + "=" * 60)
    if all_passed:
        print("✓ All tests passed!")
    else:
        print("✗ Some tests failed!")


if __name__ == "__main__":
    print("STRING SHIFT ALGORITHMS DEMONSTRATION")
    print("=" * 60)
    print()
    
    demonstrate_all_algorithms()
    
    print("\n" + "=" * 60)
    print()
    
    run_comprehensive_tests()
