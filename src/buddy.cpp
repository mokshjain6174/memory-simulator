#include "../include/buddy.h"
#include <algorithm>
#include <iomanip>

BuddyAllocator::BuddyAllocator(int size) {
    this->total_size = size;
    int max_order = (int)std::ceil(std::log2(size));
    free_lists.resize(max_order + 1);
    
    // Initially, one large free block at address 0
    free_lists[max_order].push_back(0);
}

int BuddyAllocator::get_order(int size) {
    return (int)std::ceil(std::log2(size));
}

int BuddyAllocator::buddy_malloc(int size) {
    int order = get_order(size);
    int max_order = free_lists.size() - 1;
    int current_order = order;

    // Find smallest available block
    while (current_order <= max_order && free_lists[current_order].empty()) {
        current_order++;
    }

    if (current_order > max_order) return -1; 

    // Split downwards
    int block_addr = free_lists[current_order].front();
    free_lists[current_order].pop_front();

    while (current_order > order) {
        current_order--;
        int buddy_addr = block_addr + (1 << current_order);
        free_lists[current_order].push_back(buddy_addr); // Add buddy to free list
        
    }

    allocated_blocks[block_addr] = order;
    return block_addr;
}

void BuddyAllocator::buddy_free(int address) {
    if (allocated_blocks.find(address) == allocated_blocks.end()) {
        std::cout << "Error: Invalid or double free." << std::endl;
        return;
    }

    int order = allocated_blocks[address];
    allocated_blocks.erase(address);

    // Merge upwards
    while (order < free_lists.size() - 1) {
        int buddy_addr = address ^ (1 << order); // XOR to find buddy address
        
        // Check if buddy is in the free list of the current order
        auto& list = free_lists[order];
        auto it = std::find(list.begin(), list.end(), buddy_addr);

        if (it != list.end()) {
            // Buddy is free! Remove it and merge.
            list.erase(it);
            if (buddy_addr < address) address = buddy_addr; // Keep the lower address
            order++;
        } else {
            // Buddy is not free, stop merging
            break;
        }
    }
    
    // Add merged block to the final order list
    free_lists[order].push_back(address);
}

void BuddyAllocator::dump_buddy_memory() {
    std::cout << "--- Buddy Free Lists ---" << std::endl;
    for (int i = 0; i < free_lists.size(); i++) {
        if (!free_lists[i].empty()) {
            std::cout << "Order " << i << " (" << (1 << i) << "): ";
            for (int addr : free_lists[i]) {
                std::cout << "[0x" << std::hex << addr << std::dec << "] ";
            }
            std::cout << std::endl;
        }
    }
    std::cout << "------------------------" << std::endl;
}