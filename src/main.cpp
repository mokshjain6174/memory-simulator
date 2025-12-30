#include <iostream>
#include <string>
#include <sstream>
#include "MemoryManager.h"

// Helper to convert a hex string (e.g. "0x7ff...") back to a pointer
void* stringToPtr(const std::string& addrStr) {
    unsigned long long address;
    std::stringstream ss;
    ss << std::hex << addrStr; // Tell stream to read hexadecimal
    ss >> address;
    return reinterpret_cast<void*>(address);
}

int main() {
    // 1. Initialize Memory (Default 1024 for simplicity)
    size_t initial_size = 1024;
    MemoryManager mem(initial_size);
    mem.set_strategy(BEST_FIT); // Default to Best Fit

    std::cout << "=== Memory Simulator CLI ===" << std::endl;
    std::cout << "Commands: " << std::endl;
    std::cout << "  malloc <size>   : Allocate bytes" << std::endl;
    std::cout << "  free <address>  : Free memory (use hex address from dump)" << std::endl;
    std::cout << "  dump            : Show memory map" << std::endl;
    std::cout << "  stats           : Show utilization & fragmentation" << std::endl;
    std::cout << "  exit            : Quit" << std::endl;

    std::string command;
    while (true) {
        std::cout << "\n> ";
        std::cin >> command;

        if (command == "exit") {
            break;
        }
        
        else if (command == "malloc") {
            size_t size;
            std::cin >> size; // Read the number after 'malloc'
            
            void* ptr = mem.my_malloc(size);
            if (ptr) {
                std::cout << "Allocated at address: " << ptr << std::endl;
            } else {
                std::cout << "Allocation Failed!" << std::endl;
            }
        }
        
        else if (command == "free") {
            std::string addrStr;
            std::cin >> addrStr; // Read the address string
            
            void* ptr = stringToPtr(addrStr);
            mem.my_free(ptr);
        }
        
        else if (command == "dump") {
            mem.dump_memory();
        }
        
        else if (command == "stats") {
            mem.calculate_stats();
        }
        
        else if (command == "strategy") {
            // Optional: Let user switch strategies live
            std::string mode;
            std::cin >> mode;
            if (mode == "best") mem.set_strategy(BEST_FIT);
            else if (mode == "worst") mem.set_strategy(WORST_FIT);
            else if (mode == "first") mem.set_strategy(FIRST_FIT);
            std::cout << "Strategy changed to: " << mode << std::endl;
        }

        else {
            std::cout << "Unknown command." << std::endl;
            // Clear input buffer to prevent infinite loops on bad input
            std::cin.ignore(1000, '\n'); 
        }
    }

    return 0;
}