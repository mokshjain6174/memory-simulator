#ifndef BUDDY_ALLOCATOR_H
#define BUDDY_ALLOCATOR_H

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

struct BuddyBlock {
    size_t size;    
    bool is_free;  
    BuddyBlock* next; 
};

class BuddyAllocator {
private:
    void* memory_start;
    size_t total_size;
    std::vector<BuddyBlock*> free_lists; 
    int get_order(size_t size);

public:
    BuddyAllocator(size_t size);
    ~BuddyAllocator();
    void* buddy_malloc(size_t size);
    void buddy_free(void* ptr);
    void dump_buddy_memory();
};

#endif