#ifndef MEMORY_H
#define MEMORY_H

#include <list>
#include <iostream>


struct Block {
    int start_addr;
    int size;
    bool is_free;
    int id; 
};

enum AllocationStrategy {
    FIRST_FIT,
    BEST_FIT,
    WORST_FIT
};

class MemoryManager {
private:
    int total_size;
    std::list<Block> memory_list; 
    AllocationStrategy strategy;
    int next_id = 1;

public:
    MemoryManager(int size);
    
    void set_strategy(AllocationStrategy mode);
    int my_malloc(int size);  
    void my_free(int address); 
    void dump_memory();
    void calculate_stats();
    
    // Helper
    int get_block_id(int address);
};

#endif