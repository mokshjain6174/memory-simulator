#include "BuddyAllocator.h"
#include <cstdint>
#include <iomanip>

BuddyAllocator::BuddyAllocator(size_t size) {
    this->total_size = size;
    this->memory_start = ::operator new(size);
    int max_order = std::ceil(std::log2(size));
    free_lists.resize(max_order + 1, nullptr);

    BuddyBlock* initial_block = reinterpret_cast<BuddyBlock*>(memory_start);
    initial_block->size = size;
    initial_block->is_free = true;
    initial_block->next = nullptr;

    free_lists[max_order] = initial_block;

    std::cout << "DEBUG: Buddy Allocator Init. Total Size: " << size 
              << " (Order " << max_order << ")" << std::endl;
}

BuddyAllocator::~BuddyAllocator() {
    ::operator delete(memory_start);
    std::cout << "DEBUG: Buddy Memory Destroyed." << std::endl;
}

int BuddyAllocator::get_order(size_t size) {
    return std::ceil(std::log2(size));
}

void* BuddyAllocator::buddy_malloc(size_t size) {
    size_t actual_req = size + sizeof(BuddyBlock);

    int order = get_order(actual_req);
    int max_order = free_lists.size() - 1;
    int current_order = order;
    while (current_order <= max_order && free_lists[current_order] == nullptr) {
        current_order++;
    }

    if (current_order > max_order) {
        std::cout << "DEBUG: Buddy Malloc Failed. No block large enough." << std::endl;
        return nullptr;
    }
    while (current_order > order) {

        BuddyBlock* block = free_lists[current_order];
        free_lists[current_order] = block->next;

        current_order--;
        size_t buddy_size = 1 << current_order; 

        BuddyBlock* buddy = reinterpret_cast<BuddyBlock*>(
            reinterpret_cast<char*>(block) + buddy_size
        );
        block->size = buddy_size;
        block->is_free = true;
        
        buddy->size = buddy_size;
        buddy->is_free = true;
        
        buddy->next = free_lists[current_order];
        free_lists[current_order] = buddy;
        
        block->next = free_lists[current_order];
        free_lists[current_order] = block;
        
        std::cout << "DEBUG: Split order " << (current_order+1) << " -> two order " << current_order << std::endl;
    }

    
    BuddyBlock* allocated = free_lists[order];
    free_lists[order] = allocated->next;

    allocated->is_free = false;
    return reinterpret_cast<void*>(reinterpret_cast<char*>(allocated) + sizeof(BuddyBlock));
}
void BuddyAllocator::buddy_free(void* ptr) {
    if (!ptr) return;
    BuddyBlock* block = reinterpret_cast<BuddyBlock*>(static_cast<char*>(ptr) - sizeof(BuddyBlock));
    size_t size = block->size;
    block->is_free = true;
    std::cout << "DEBUG: Freeing block of size " << size << std::endl;

    while (size < total_size) {
        
        size_t offset = reinterpret_cast<char*>(block) - reinterpret_cast<char*>(memory_start);

        size_t buddy_offset = offset ^ size;
        
        BuddyBlock* buddy = reinterpret_cast<BuddyBlock*>(
            reinterpret_cast<char*>(memory_start) + buddy_offset
        );

        if (!buddy->is_free || buddy->size != size) {
            break; 
        }
        std::cout << "DEBUG: Merging with buddy at offset " << buddy_offset << std::endl;
        int order = get_order(size);
        BuddyBlock* curr = free_lists[order];
        BuddyBlock* prev = nullptr;
        while (curr && curr != buddy) {
            prev = curr;
            curr = curr->next;
        }
        if (prev) prev->next = buddy->next;
        else free_lists[order] = buddy->next;

        if (buddy < block) {
            block = buddy;
        }

        size *= 2;
        block->size = size;
    }

    int final_order = get_order(size);
    block->next = free_lists[final_order];
    free_lists[final_order] = block;
}
void BuddyAllocator::dump_buddy_memory() {
    std::cout << "--- Buddy Free Lists ---" << std::endl;
    for (size_t i = 0; i < free_lists.size(); i++) {
        std::cout << "Order " << i << " (" << (1 << i) << " bytes): ";
        BuddyBlock* curr = free_lists[i];
        if (!curr) std::cout << "Empty";
        while (curr) {
            std::cout << "[Addr: " << curr << "] -> ";
            curr = curr->next;
        }
        std::cout << std::endl;
    }
    std::cout << "------------------------" << std::endl;
}