#ifndef BUDDY_H
#define BUDDY_H

#include <vector>
#include <list>
#include <unordered_map>
#include <cmath>
#include <iostream>

class BuddyAllocator {
private:
    int total_size;
    std::vector<std::list<int>> free_lists; 
    
    std::unordered_map<int, int> allocated_blocks; 

    int get_order(int size);

public:
    BuddyAllocator(int size);
    int buddy_malloc(int size);
    void buddy_free(int address);
    void dump_buddy_memory();
};

#endif