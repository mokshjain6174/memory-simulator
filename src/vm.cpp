#include "../include/vm.h"

VirtualMemory::VirtualMemory(int phys_mem_size, int p_size) {
    this->page_size = p_size;
    this->num_frames = phys_mem_size / p_size;
    this->frame_owner.assign(num_frames, -1);
}

void VirtualMemory::create_process(int pid, int virtual_size) {
    int num_pages = (virtual_size + page_size - 1) / page_size;
    std::vector<PageTableEntry> pt(num_pages, {false, -1});
    page_tables[pid] = pt;
    std::cout << "VM: Process " << pid << " created (" << num_pages << " pages)." << std::endl;
}

int VirtualMemory::access(int pid, int virtual_addr) {
    if (page_tables.find(pid) == page_tables.end()) return -1;

    int page_num = virtual_addr / page_size;
    int offset = virtual_addr % page_size;

    if (page_num >= page_tables[pid].size()) return -1;

    PageTableEntry& entry = page_tables[pid][page_num];

    if (entry.valid) {
        page_hits++;
        return (entry.frame_number * page_size) + offset;
    }

    // Page Fault
    page_faults++;
    std::cout << "VM: Page Fault (PID " << pid << ", Page " << page_num << ")" << std::endl;

    // Find victim frame (Simple First-Free or FIFO 0)
    int victim_frame = -1;
    for (int i = 0; i < num_frames; i++) {
        if (frame_owner[i] == -1) {
            victim_frame = i;
            break;
        }
    }
    
    // Eviction Logic (Simplified: Always steal frame 0 if full)
    if (victim_frame == -1) {
        victim_frame = 0;
        int old_pid = frame_owner[0];
        if (old_pid != -1) {
            // Invalidate old owner's page (Linear scan for simplicity in simulation)
            for (auto& entry : page_tables[old_pid]) {
                if (entry.frame_number == 0) entry.valid = false;
            }
        }
    }

    frame_owner[victim_frame] = pid;
    entry.valid = true;
    entry.frame_number = victim_frame;

    return (victim_frame * page_size) + offset;
}

void VirtualMemory::print_stats() {
    std::cout << "VM Stats -> Hits: " << page_hits << " | Faults: " << page_faults << std::endl;
}