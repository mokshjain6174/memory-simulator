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

// 1. ALLOCATION (First Fit Strategy)
void* MemoryManager::my_malloc(size_t size) {
    std::cout << "DEBUG: Requesting allocation of " << size << " bytes..." << std::endl;
    
    Block* current = free_list_head;

    while (current != nullptr) {
        // STRATEGY: First Fit [cite: 22]
        // Check if the block is FREE and has enough SIZE
        if (current->is_free && current->size >= size) {
            
            //CHECK:Can we split this block?[cite: 27]
            //We need enough space for the requested data + a NEW header + at least 1 byte of data , so that split is usefull
            if (current->size > size + sizeof(Block)) {
                
                // A. Calculate address of the new neighbor block
                // (Current Address + Header Size + Request Size)
                Block* new_block = reinterpret_cast<Block*>(
                    reinterpret_cast<char*>(current) + sizeof(Block) + size
                );

                // B. Setup the new block's metadata
                new_block->size = current->size - size - sizeof(Block);
                new_block->is_free = true;
                new_block->next = current->next; // Insert into list

                // C. Update the current block (User gets this part)
                current->size = size;
                current->is_free = false;
                current->next = new_block; // Point to our new neighbor

                std::cout << "DEBUG: Split block. New free hole size: " << new_block->size << std::endl;
            } else {
                // Exact fit (or too small to split usefuly)
                current->is_free = false;
                std::cout << "DEBUG: Exact fit used. No split." << std::endl;
            }

            // RETURN: Pointer to the DATA, not the header!
            return reinterpret_cast<void*>(reinterpret_cast<char*>(current) + sizeof(Block));
        }

        // Advance to next block , (iterations)
        current = current->next;
    }

    std::cout << "DEBUG: Allocation failed! No suitable block found." << std::endl;
    return nullptr;
}

// 2. DEALLOCATION
void MemoryManager::my_free(void* ptr) {
    if (ptr == nullptr) return;

    // A. Backtrack from the User Pointer to the Header
    Block* block_to_free = reinterpret_cast<Block*>(static_cast<char*>(ptr) - sizeof(Block));

    // B. Mark as free
    block_to_free->is_free = true;
    std::cout << "DEBUG: Freed block of size " << block_to_free->size << std::endl;

    // C. Clean up fragmentation 
    coalesce();
}

// 3. COALESCING (Merging neighbor free blocks into single bigger block)
void MemoryManager::coalesce() {
    Block* current = free_list_head;

    while (current != nullptr && current->next != nullptr) {
        
        // If THIS block is free AND the NEXT block is free...
        if (current->is_free && current->next->is_free) {
            
            //make single bigger block
            //new size = my size + next header size + Next Data Size
            current->size += sizeof(Block) + current->next->size;
            
            // skip the next block (removing it from the list)
            current->next = current->next->next;
            
            std::cout << "DEBUG: Merged adjacent free blocks!" << std::endl;
            
            // WARNING: Do not move 'current' forward! 
            // We must check if we can merge again with the *new* neighbor.
        } else {
            current = current->next;
        }
    }
}