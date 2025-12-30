#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <cstddef> 

enum AllocationStrategy {
    FIRST_FIT,
    BEST_FIT,
    WORST_FIT
};

struct Block {
    int id;           
    size_t size;
    bool is_free;
    Block* next;
};

class MemoryManager {
private:
    void* memory_start;
    size_t total_size;
    Block* free_list_head;
    AllocationStrategy strategy;
    int next_id = 1;

public:
    MemoryManager(size_t size);
    ~MemoryManager();
    void dump_memory();
    void* my_malloc(size_t size);
    void my_free(void* ptr);
    void coalesce();
    void calculate_stats();
    void set_strategy(AllocationStrategy mode);
    int get_block_id(void* ptr);
};

#endif