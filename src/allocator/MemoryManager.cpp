#include <iostream>
#include <new> // For std::bad_alloc
#include "MemoryManager.h" // The compiler will find this using the -I flag

MemoryManager::MemoryManager(size_t size) {
    this->total_size = size;
    
    //Allocate the "Physical RAM"
    try {
        this->memory_start = ::operator new(size);
    } catch (const std::bad_alloc& e) {
        std::cerr << "Fatal Error: Could not allocate simulated RAM." << std::endl;
        throw;
    }

    //Initialize the Free List
    // We cast the start of the raw memory to a Block pointer.
    this->free_list_head = reinterpret_cast<Block*>(memory_start);

    //Set the metadata
    //The usable size is Total RAM minus the size of the header itself.
    this->free_list_head->size = size - sizeof(Block);
    this->free_list_head->is_free = true;
    this->free_list_head->next = nullptr; 

    std::cout << "DEBUG: Memory Initialized. Start Address: " << memory_start 
              << " | Metadata Size: " << sizeof(Block) << " bytes" << std::endl;
}

MemoryManager::~MemoryManager() {
    // Clean up the raw memory when the simulator closes
    ::operator delete(memory_start);
    std::cout << "DEBUG: Memory Destroyed." << std::endl;
}

void MemoryManager::dump_memory() {
    std::cout << "--- Memory Map ---" << std::endl;
    Block* current = free_list_head;
    
    while (current != nullptr) {
        std::cout << "[Block @ " << current << "] "
                  << (current->is_free ? "FREE" : "USED")
                  << " | Size: " << current->size 
                  << " bytes" << std::endl;
        current = current->next;
    }
    std::cout << "------------------" << std::endl;
}


void* MemoryManager::my_malloc(size_t size) { return nullptr; }
void MemoryManager::my_free(void* ptr) { }