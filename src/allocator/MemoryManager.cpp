#include <iostream>
#include <new> 
#include "MemoryManager.h" 

MemoryManager::MemoryManager(size_t size){
    this->total_size=size;
    try{
        this->memory_start= ::operator new(size);
    }catch(const std::bad_alloc& e){
        std::cerr << "Fatal Error: Could not allocate simulated RAM." << std::endl;
        throw;
    }

    this->free_list_head=reinterpret_cast<Block*>(memory_start);
    this->free_list_head->size=size-sizeof(Block);
    this->free_list_head->is_free=true;
    this->free_list_head->next = nullptr;
    std::cout << "DEBUG: Memory Initialized. Start Address: " << memory_start 
              << " | Metadata Size: " << sizeof(Block) << " bytes" << std::endl;
}

MemoryManager::~MemoryManager() {
    // Clean up the raw memory when the simulator closes
    ::operator delete(memory_start);
    std::cout << "DEBUG: Memory Destroyed." << std::endl;
}
void MemoryManager::dump_memory(){
    std::cout << "--- Memory Map ---" << std::endl;
    Block* current=free_list_head;
    while(current!=nullptr){
        std::cout << "[Block @ " << current << "] ";
        if (current->is_free) {
            std::cout << "FREE";
        } else {
            std::cout << "USED (id=" << current->id << ")";
        }
        std::cout << " | Size: " << current->size << " bytes" << std::endl;
        current = current->next;
    }
    std::cout << "------------------" << std::endl;
}


//setting strategy for memory allocation
void MemoryManager::set_strategy(AllocationStrategy mode) {
    this->strategy = mode;
}

//Allocator
void* MemoryManager::my_malloc(size_t size) {
    
    Block* current = free_list_head;
    Block* best_block = nullptr;

    while (current != nullptr) {
        if(current->is_free && current->size>=size){
            if(strategy==FIRST_FIT){
                best_block=current;
                break;
            }else if(strategy==BEST_FIT){
                if(best_block==nullptr || current->size < best_block->size){
                    best_block=current;
                }
            }else if(strategy==WORST_FIT){
                if(best_block==nullptr || current->size > best_block->size){
                    best_block=current;
                }
            }
        }
        current=current->next;
        
    }

    if (best_block == nullptr) {
        return nullptr;
    }

    // --- ALLOCATION & ID ASSIGNMENT ---
    
    // Split Logic
    if(best_block->size > size + sizeof(Block)){
        Block* new_block=reinterpret_cast<Block*>(reinterpret_cast<char*>
        (best_block)+sizeof(Block) + size);
        
        new_block->id=0;
        new_block->is_free=true;
        new_block->size=best_block->size - size - sizeof(Block);
        new_block->next=best_block->next;

        best_block->size=size;
        best_block->is_free=false;
        best_block->next=new_block;
    }else{
        best_block->is_free=false;
    }

    best_block->id = this->next_id++; 
    
    return reinterpret_cast<void*>(reinterpret_cast<char*>(best_block) + sizeof(Block));
}

// 2. DEALLOCATION
void MemoryManager::my_free(void* ptr) {
    if (ptr == nullptr) return;
    //Backtrack from the User Pointer to the Header
    Block* block_to_free = reinterpret_cast<Block*>(static_cast<char*>(ptr) - sizeof(Block));

    //Mark as free
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
            
            // We must check if we can merge again with the *new* neighbor.
        } else {
            current = current->next;
        }
    }
}

void MemoryManager::calculate_stats() {
    size_t total_free = 0;
    size_t total_used = 0;
    size_t largest_free_block = 0;
    size_t free_blocks_count = 0;

    Block* current = free_list_head;

    while (current != nullptr) {
        if (current->is_free) {
            total_free += current->size;
            free_blocks_count++;
            if (current->size > largest_free_block) {
                largest_free_block = current->size;
            }
        } else {
            total_used += current->size;
        }
        current = current->next;
    }

    size_t total_mem = total_free + total_used; // Approximate (excludes headers)
    
    // Utilization: Percentage of "Data" space currently occupied
    double utilization = (total_mem > 0) ? (static_cast<double>(total_used) / total_mem) * 100.0 : 0.0;

    // Fragmentation: (Total Free - Largest Chunk) / Total Free
    // This tells us how much of our free RAM is "trapped" in small unusable holes.
    double fragmentation = (total_free > 0) ? 
        (static_cast<double>(total_free - largest_free_block) / total_free) * 100.0 : 0.0;

    std::cout << "\n=== Memory Statistics ===" << std::endl;
    std::cout << "Total Memory (Data): " << total_mem << " bytes" << std::endl;
    std::cout << "Used Memory:         " << total_used << " bytes" << std::endl;
    std::cout << "Free Memory:         " << total_free << " bytes" << std::endl;
    std::cout << "Free Blocks Count:   " << free_blocks_count << std::endl;
    std::cout << "Utilization:         " << utilization << "%" << std::endl;
    std::cout << "Ext. Fragmentation:  " << fragmentation << "%" << std::endl;
    std::cout << "=========================" << std::endl;
}

int MemoryManager::get_block_id(void* ptr) {
    if (!ptr) return -1;
    // Go back 24 bytes (or sizeof(Block)) to find the header
    Block* block = reinterpret_cast<Block*>(static_cast<char*>(ptr) - sizeof(Block));
    return block->id;
}