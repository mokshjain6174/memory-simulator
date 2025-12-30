#include <iostream>
#include <string>
#include <sstream>
#include "MemoryManager.h"
#include "buddy/BuddyAllocator.h" 

void* stringToPtr(const std::string& addrStr) {
    unsigned long long address;
    std::stringstream ss;
    ss << std::hex << addrStr; 
    ss >> address;
    return reinterpret_cast<void*>(address);
}

int main() {
    std::cout << "=== Buddy Allocator Test ===" << std::endl;
    BuddyAllocator buddy(1024);
    buddy.dump_buddy_memory();

    std::cout << "\n> malloc(100)" << std::endl;
    
    void* p1 = buddy.buddy_malloc(100); 
    buddy.dump_buddy_memory();
    std::cout << "\n> malloc(100)" << std::endl;

    void* p2 = buddy.buddy_malloc(100);
    buddy.dump_buddy_memory();
    std::cout << "\n> free(p1)" << std::endl;
    buddy.buddy_free(p1);
    buddy.dump_buddy_memory();

    std::cout << "\n> free(p2) (Should Coalesce)" << std::endl;

    buddy.buddy_free(p2);
    buddy.dump_buddy_memory();

    return 0;
}