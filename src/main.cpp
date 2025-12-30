#include <iostream>
#include "MemoryManager.h"

int main() {
    std::cout << "=== Memory Strategy Test ===" << std::endl;
    MemoryManager mem(1024);
    
    // 1. SETUP: Create holes 
    // We reduce the sizes slightly to ensure they all fit in 1024 bytes.
    
    void* p1 = mem.my_malloc(200);
    void* p2 = mem.my_malloc(300); 
    void* p3 = mem.my_malloc(200);
    void* p4 = mem.my_malloc(50);  
    
    // Create the holes
    std::cout << "\n--- Creating Holes ---" << std::endl;
    mem.my_free(p2); // Free the 300 block (Big Hole)
    mem.my_free(p4); // Free the 50 block (Small Hole)
    
    std::cout << "\n--- Memory State Before Allocation ---" << std::endl;
    mem.dump_memory();

    // 2. TEST: Ask for 40 bytes using BEST FIT
    // Options: 
    // - Hole A: 300 bytes
    // - Hole B: 50 bytes
    // Best Fit MUST pick Hole B (50 bytes).
    std::cout << "\n--- Requesting 40 bytes (Strategy: BEST FIT) ---" << std::endl;
    mem.set_strategy(BEST_FIT);
    
    void* p_test = mem.my_malloc(40);
    mem.dump_memory();

    return 0;
}