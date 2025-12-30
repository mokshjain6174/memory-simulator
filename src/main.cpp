#include <iostream>
#include "MemoryManager.h"

int main() {
    MemoryManager mem(1024); // 1000 bytes usable

    std::cout << "\n=== Test 1: Allocating (Splitting) ===" << std::endl;
    // This should take the first 100 bytes + 24 header.
    // Remaining free: 1000 - 124 = 876 bytes.
    void* p1 = mem.my_malloc(100);
    mem.dump_memory();

    void* p2 = mem.my_malloc(200);
    mem.dump_memory();

    void* p3 = mem.my_malloc(100);
    mem.dump_memory();

    std::cout << "\n=== Test 2: Freeing Middle Block (Fragmentation) ===" << std::endl;
    mem.my_free(p2); 
    // You should see: USED -> FREE (Hole) -> USED -> FREE (Tail)
    mem.dump_memory();

    std::cout << "\n=== Test 3: Coalescing (Merging) ===" << std::endl;
    mem.my_free(p1);
    mem.my_free(p3);
    // Now everything should merge back into one big FREE block
    mem.dump_memory();

    return 0;
}