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
// 1. Helper Setter
void MemoryManager::set_strategy(AllocationStrategy mode) {
    this->strategy = mode;
}

// 2. The Smart Allocator
void* MemoryManager::my_malloc(size_t size) {
    std::cout << "DEBUG: Allocating " << size << " bytes..." << std::endl;
    
    Block* best_block = nullptr;
    Block* current = free_list_head;

    // --- SEARCH PHASE ---
    while (current != nullptr) {
        
        // We only care about blocks that are FREE and BIG ENOUGH
        if (current->is_free && current->size >= size) {
            
            // Strategy 1: FIRST FIT
            if (strategy == FIRST_FIT) {
                best_block = current;
                break; // Stop searching immediately
            }

            // Strategy 2: BEST FIT
            // We want the SMALLEST block that works
            else if (strategy == BEST_FIT) {
                if (best_block == nullptr || current->size < best_block->size) {
                    best_block = current;
                }
            }

            // Strategy 3: WORST FIT
            // We want the LARGEST block available
            else if (strategy == WORST_FIT) {
                if (best_block == nullptr || current->size > best_block->size) {
                    best_block = current;
                }
            }
        }
        current = current->next; // Move to next node
    }

    // --- EXECUTION PHASE ---
    // If we didn't find ANY block, return nullptr
    if (best_block == nullptr) {
        std::cout << "DEBUG: Allocation failed. No memory." << std::endl;
        return nullptr;
    }

    // If we found a block, check if we need to SPLIT it
    if (best_block->size > size + sizeof(Block)) {
        Block* new_block = reinterpret_cast<Block*>(
            reinterpret_cast<char*>(best_block) + sizeof(Block) + size
        );
        new_block->size = best_block->size - size - sizeof(Block);
        new_block->is_free = true;
        new_block->next = best_block->next;

        best_block->size = size;
        best_block->is_free = false;
        best_block->next = new_block;
        std::cout << "DEBUG: Strategy selected block. Split performed." << std::endl;
    } else {
        best_block->is_free = false;
        std::cout << "DEBUG: Strategy selected block. Exact fit used." << std::endl;
    }

    return reinterpret_cast<void*>(reinterpret_cast<char*>(best_block) + sizeof(Block));
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
            //new size = my size + next header size+Next Data Size
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