#ifndef clox_memory_h
#define clox_memory_h

#include "common.h"

#define GROW_CAPACITY(capacity) ((capacity) < 8 ? 8 : (capacity) * 2)

#define GROW_ARRAY(type, pointer, oldCount, newCount)                          \
    (type*)reallocate(pointer, sizeof(type) * (oldCount),                      \
                      sizeof(type) * (newCount))

#define FREE_ARRAY(type, pointer, oldCount)                                    \
    reallocate(pointer, sizeof(type) * (oldCount), 0)

// NOTE: All what's related to memory management in this Clox is managed by this
// only function
// old size = 0, new size = non-zeor => allocate new block old
// size = non-zero, new size = 0 => free allocation old size = non-zero, new
// size = smaller than old size => shrink existing allocation old size =
// non-zero, new size = larger than old size => grow existing allocation
void* reallocate(void* pointer, size_t oldSize, size_t newSize);

#endif
