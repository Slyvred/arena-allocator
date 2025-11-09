# Arena Allocator

This repository contains a simple C++ arena allocator implementation using a linked list to allow dynamic arena expansion.

## What is an arena allocator?

An arena allocator pre-allocates a large chunk of memory and performs all allocations within that chunk.
The goal is to avoid frequent calls to `new`, `malloc`, `delete`, or `free`, which are relatively expensive.
By allocating inside our pre-allocated memory region, we significantly reduce allocation overhead.

This approach allows fast allocations and deallocations — typically requiring only **one** `new` and **one** `delete` per memory block (i.e., per linked-list node).

## Implementation details

The arena is implemented using a doubly linked list (`std::list`) that stores multiple buffers.
Each buffer represents its own contiguous memory region and capacity. The capacity doubles each time a new buffer is created, which helps minimize the number of heap allocations.

When allocating an object:
1. The allocator tries to place it inside the current buffer (using pointer alignment).
2. If the allocation doesn’t fit:
   - A new buffer is allocated with doubled capacity, and the allocation is retried.
   - If a next buffer already exists (for example after calling `Arena::reset()`), the allocator simply moves to that buffer and reuses it.

The `Arena::reset()` function resets the allocation offset and buffer index, effectively marking all buffers as reusable without freeing any memory.

## Example use

```c++
int main(int argc, char** argv) {
    Arena arena = Arena(8); // Create a new arena with a size of 8 bytes

    // Allocate 3 2D points of 8 bytes
    // 3 * 8 = 24 = 8 + 16 --> Will require one new block allocation
    Point2D* points[3];
    for (int i = 0; i < 3; i++) {
        points[i] = arena.make<Point2D>(2*i, 3*i);
        std::cout << *points[i] << "\n";
    }

    // We reset the arena, marking all the blocks availble for overwrite
    arena.reset();

    // Allocate 3 2D points of 8 bytes again but with no allocation since the new block is already there
    for (int i = 0; i < 3; i++) {
        points[i] = arena.make<Point2D>(20*i+1, 30*i+1);
        std::cout << *points[i] << "\n";
    }
}
```

Output:
```console
[DEBUG - 11/09/25 22:21:46] Created arena of capaticy 8, at 0x5571c1871350
Point2D{
 x: 0,
 y: 0
}
[DEBUG - 11/09/25 22:21:46] Created new buffer of capacity 16 at 0x5571c1871a90
Point2D{
 x: 2,
 y: 3
}
Point2D{
 x: 4,
 y: 6
}
[DEBUG - 11/09/25 22:21:46] Arena was reset
Point2D{
 x: 1,
 y: 1
}
Point2D{
 x: 21,
 y: 31
}
Point2D{
 x: 41,
 y: 61
}
```

Notice that for a total of 6 allocations we only called `malloc()` and `free()` two times each (1 time per block). If we manually allocated these in the heap using `new` or `malloc()` the sum of the calls would have been 12 instead of 4, that's 3 times as much !
