#ifndef VM_H
#define VM_H

#include <vector>
#include <unordered_map>
#include <iostream>

struct PageTableEntry {
    bool valid;
    int frame_number;
};

class VirtualMemory {
private:
    int page_size;
    int num_frames;
    
    // Simulation: frame_owner[frame_index] = PID (or -1 if free)
    std::vector<int> frame_owner; 
    
    // Page Tables: PID -> Vector of Entries
    std::unordered_map<int, std::vector<PageTableEntry>> page_tables;

    int page_faults = 0;
    int page_hits = 0;

public:
    VirtualMemory(int phys_mem_size, int page_size);
    void create_process(int pid, int virtual_size);
    int access(int pid, int virtual_addr); // Returns Physical Address (int)
    void print_stats();
};

#endif