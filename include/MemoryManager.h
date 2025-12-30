#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <cstddef> 

// The metadata header for every memory block
struct Block {
    size_t size;       // Size of the data part (excluding this header)
    bool is_free;      // Is this block free or used?
    Block* next;       // Pointer to the next block in the list
};

class MemoryManager {
private:
    void* memory_start;      // Pointer to the start of our simulated RAM
    size_t total_size;       // Total size of the simulated RAM
    Block* free_list_head;   // Head of the linked list of blocks

public:
    // Constructor: Initializes the memory pool
    MemoryManager(size_t size);

    // Destructor: Cleans up the memory pool
    ~MemoryManager();

    // Helper to see what the memory looks like
    void dump_memory();

    // (Phase 2 functions will go here later)
    void* my_malloc(size_t size);
    void my_free(void* ptr);
};

#endif