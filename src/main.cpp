#include <iostream>
#include <string>
#include <sstream>
#include "MemoryManager.h"

void* stringToPtr(const std::string& addrStr) {
    unsigned long long address;
    std::stringstream ss;
    ss << std::hex << addrStr; 
    ss >> address;
    return reinterpret_cast<void*>(address);
}

int main() {
    
    MemoryManager* mem = new MemoryManager(1024);
    mem->set_strategy(FIRST_FIT);

    std::cout << "=== Memory Simulator CLI ===" << std::endl;
    std::cout << "Commands:" << std::endl;
    std::cout << "  init memory <size>       : Restart with new size" << std::endl;
    std::cout << "  set allocator <mode>     : first / best / worst" << std::endl;
    std::cout << "  malloc <size>            : Allocate bytes" << std::endl;
    std::cout << "  free <address>           : Free memory (use hex address)" << std::endl;
    std::cout << "  dump memory              : Show map" << std::endl;
    std::cout << "  stats                    : Show usage metrics" << std::endl;
    std::cout << "  exit                     : Quit" << std::endl;

    std::string command;
    while (true) {
        std::cout << "\n> ";
        std::cin >> command; 

        if (command == "exit") {
            break;
        }

        // COMMAND: init memory <size>
        else if (command == "init") {
            std::string subcmd;
            size_t size;
            std::cin >> subcmd >> size; // Read "memory" and "1024"
            
            delete mem; // Destroy old memory
            mem = new MemoryManager(size);
            std::cout << "Memory re-initialized with " << size << " bytes." << std::endl;
        }

        else if (command == "set") {
            std::string subcmd, mode;
            std::cin >> subcmd >> mode; // Read "allocator" and "best_fit"
            
            if (mode == "first") {
                mem->set_strategy(FIRST_FIT);
                std::cout << "Strategy set to First Fit" << std::endl;
            } else if (mode == "best") {
                mem->set_strategy(BEST_FIT);
                std::cout << "Strategy set to Best Fit" << std::endl;
            } else if (mode == "worst") {
                mem->set_strategy(WORST_FIT);
                std::cout << "Strategy set to Worst Fit" << std::endl;
            } else {
                std::cout << "Unknown strategy. Use: first, best, or worst." << std::endl;
            }
        }

        
        else if (command == "malloc") {
            size_t size;
            std::cin >> size;
            void* ptr = mem->my_malloc(size);
            
            if (ptr) {
                int id = mem->get_block_id(ptr);
                // Matches format: "Allocated block id=1 at address=0x..."
                std::cout << "Allocated block id=" << id << " at address=" << ptr << std::endl;
            } else {
                std::cout << "Allocation Failed!" << std::endl;
            }
        }
        

        
        else if (command == "free") {
            std::string addrStr;
            std::cin >> addrStr;
            void* ptr = stringToPtr(addrStr);
            mem->my_free(ptr);
        }

        else if (command == "dump") {
            std::string subcmd; 
            std::cin >> subcmd; 
            mem->dump_memory();
        }

        // COMMAND: stats
        else if (command == "stats") {
            mem->calculate_stats();
        }

        else {
            std::cout << "Unknown command." << std::endl;
            // Clear input buffer to prevent loops
            std::cin.ignore(1000, '\n'); 
        }
    }

    delete mem;
    return 0;
}