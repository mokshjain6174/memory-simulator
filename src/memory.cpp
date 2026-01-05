#include "../include/memory.h"
#include <iomanip>

MemoryManager::MemoryManager(int size) {
    this->total_size = size;
    // Initialize memory as one giant free block
    Block initial_block = {0, size, true, -1};
    memory_list.push_back(initial_block);
    strategy = FIRST_FIT;
}

void MemoryManager::set_strategy(AllocationStrategy mode) {
    this->strategy = mode;
}

void MemoryManager::dump_memory() {
    std::cout << "--- Linear Memory Map ---" << std::endl;
    for (const auto& block : memory_list) {
        std::cout << "[Addr: " << std::setw(4) << block.start_addr 
                  << " | Size: " << std::setw(4) << block.size << "] ";
        if (block.is_free) {
            std::cout << "FREE" << std::endl;
        } else {
            std::cout << "USED (id=" << block.id << ")" << std::endl;
        }
    }
    std::cout << "-------------------------" << std::endl;
}

int MemoryManager::my_malloc(int size) {
    auto best_it = memory_list.end();
    
    // Find a block based on strategy
    for (auto it = memory_list.begin(); it != memory_list.end(); ++it) {
        if (it->is_free && it->size >= size) {
            if (strategy == FIRST_FIT) {
                best_it = it;
                break;
            } else if (strategy == BEST_FIT) {
                if (best_it == memory_list.end() || it->size < best_it->size)
                    best_it = it;
            } else if (strategy == WORST_FIT) {
                if (best_it == memory_list.end() || it->size > best_it->size)
                    best_it = it;
            }
        }
    }

    if (best_it == memory_list.end()) return -1; // Allocation failed

    // Allocation Logic
    int alloc_start = best_it->start_addr;
    
    if (best_it->size == size) {
        // Perfect fit
        best_it->is_free = false;
        best_it->id = next_id++;
    } else {
        // Split block
        Block new_used_block = {best_it->start_addr, size, false, next_id++};
        Block remaining_free_block = {best_it->start_addr + size, best_it->size - size, true, -1};
        
        // Replace old block with two new blocks
        best_it = memory_list.erase(best_it);
        memory_list.insert(best_it, new_used_block);
        memory_list.insert(best_it, remaining_free_block);
    }
    
    return alloc_start;
}

void MemoryManager::my_free(int address) {
    for (auto it = memory_list.begin(); it != memory_list.end(); ++it) {
        if (it->start_addr == address) {
            if (it->is_free) {
                std::cout << "Error: Block already free." << std::endl;
                return;
            }
            it->is_free = true;
            it->id = -1;
            
            // Coalesce (Merge) Logic
            // Check next
            auto next = std::next(it);
            if (next != memory_list.end() && next->is_free) {
                it->size += next->size;
                memory_list.erase(next);
            }
            
            // Check prev
            if (it != memory_list.begin()) {
                auto prev = std::prev(it);
                if (prev->is_free) {
                    prev->size += it->size;
                    memory_list.erase(it);
                }
            }
            return;
        }
    }
    std::cout << "Error: Invalid address." << std::endl;
}

void MemoryManager::calculate_stats() {
    int total_free = 0;
    int total_used = 0;
    for (const auto& b : memory_list) {
        if (b.is_free) total_free += b.size;
        else total_used += b.size;
    }
    std::cout << "Total: " << total_size << " | Used: " << total_used 
              << " | Free: " << total_free << std::endl;
}