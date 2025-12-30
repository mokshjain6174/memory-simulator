#include <iostream>
#include "MemoryManager.h" // The compiler will find this using the -I flag

int main() {
    std::cout << "Welcome to Memory Simulator!" << std::endl;

    
    size_t memory_size = 1024;
    MemoryManager mem(memory_size);

    mem.dump_memory();

    return 0;
}