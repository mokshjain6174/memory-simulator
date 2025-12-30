#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <cstddef> // For size_t

// 1. NEW: Define the strategies available
enum AllocationStrategy {
    FIRST_FIT,
    BEST_FIT,
    WORST_FIT
};

struct Block {
    size_t size;
    bool is_free;
    Block* next;
};

class MemoryManager {
private:
    void* memory_start;
    size_t total_size;
    Block* free_list_head;
    
    // 2. NEW: Variable to store the current active strategy
    AllocationStrategy strategy = FIRST_FIT; 

public:
    MemoryManager(size_t size);
    ~MemoryManager();
    void dump_memory();
    void* my_malloc(size_t size);
    void my_free(void* ptr);
    void coalesce();

    // 3. NEW: Setter function to change strategy
    void set_strategy(AllocationStrategy mode);
    // Calculates and prints statistics
    void calculate_stats();
};

#endif