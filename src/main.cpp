#include <iostream>
#include <string>
#include <limits> 
#include <iomanip>

#include "../include/memory.h"
#include "../include/buddy.h"
#include "../include/cache.h"
#include "../include/vm.h"

using namespace std;

// ==========================================
//    GLOBAL VARIABLES 
// ==========================================

BuddyAllocator* sys_buddy = nullptr;
Cache* primary_cache = nullptr;    
Cache* secondary_cache = nullptr;  

int config_ram_size = 0;   
int config_page_size = 0;  
int system_frame_count = 0; 

enum AllocationStrategy {
    STRAT_UNSET,
    STRAT_LINEAR,
    STRAT_BUDDY
};

AllocationStrategy current_strategy = STRAT_UNSET; 

// ==========================================
//    HELPER FUNCTIONS
// ==========================================

void perform_memory_lookup(int target_addr) {
    if (target_addr < 0) return;

    if (primary_cache->access(target_addr)) {
        total_cycles += l1_penalty;
        cout << "L1 hit!\n"; // Matched friend's string
        return;
    }

    total_cycles += l1_penalty;

    if (secondary_cache->access(target_addr)) {
        total_cycles += l2_penalty;
        cout << "L1 miss. L2 hit.\n"; // Matched friend's string
        return;
    }

    total_cycles += l2_penalty + memory_penalty;
    cout << "L1 miss. L2 miss. Accessing main memory.\n"; // Matched friend's string
}

void flush_input() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

// ==========================================
//    UI / MENU FUNCTIONS
// ==========================================

void ui_line() {
    cout << "--------------------------------------------------------\n";
}

void show_main_menu() {
    cout << "\n=== OS MEMORY SIMULATOR ===\n";
    cout << "  [1] Configure System (RAM & Page Size)\n";
    cout << "  [2] Perform Allocation/Free\n";
    cout << "  [3] Virtual Memory Management\n";
    cout << "  [4] View Statistics & Dump\n";
    cout << "  [5] Exit Application\n";
    cout << "\nInput Choice: ";
}

void show_vm_submenu() {
    cout << "\n   >>> Virtual Memory Controls <<<\n";
    cout << "   [1] Init Process (vm_init)\n";
    cout << "   [2] Translate & Access Address\n";
    cout << "   [3] Inspect Page Table\n";
    cout << "   [4] Back\n";
    cout << "\n   Input Choice: ";
}

// Rewritten to match ALL data points from friend's code
void print_system_report() {
    cout << "=======STATISTICS=======\n";

    // 1. MEMORY STATS
    cout << "\n----- Memory -----\n";
    if (current_strategy == STRAT_LINEAR) {
        cout << "Allocator Type: Linear (FF/BF/WF)\n";
        
        int frag_int = internal_fragmentation();
        int frag_ext = external_fragmentation();
        
        int total = config_ram_size;
        int free_mem = 0;
        for (auto &blk : memory_blocks) if (blk.free) free_mem += blk.size;

        double int_pct = total ? (frag_int * 100.0 / total) : 0.0;
        double ext_pct = free_mem ? (frag_ext * 100.0 / free_mem) : 0.0;
        
        cout << "Internal Fragmentation: " << frag_int << " bytes (" << int_pct << "% of total memory)\n";
        cout << "External Fragmentation: " << frag_ext << " bytes (" << ext_pct << "% of free memory)\n";
        cout << "Memory Utilization: " << memory_utilization() << "%\n";
        
        allocation_stats(); 
    } 
    else if (current_strategy == STRAT_BUDDY) {
        cout << "Allocator Type: Buddy System\n";
        
        int used = sys_buddy->get_used_memory();
        int total = config_ram_size;
        int internal = sys_buddy->get_internal_fragmentation();
        int free_mem = total - used;
        
        double int_pct = used ? (internal * 100.0 / used) : 0.0;
        double util_pct = total ? (used * 100.0 / total) : 0.0;

        cout << "Total Memory: " << total << " bytes\n";
        cout << "Used Memory: " << used << " bytes\n";
        cout << "Free Memory: " << free_mem << " bytes\n";

        cout << "Internal Fragmentation: " << internal << " bytes (" << int_pct << "% of allocated memory)\n";
        cout << "External Fragmentation: 0 bytes (0%)\n";
        cout << "Memory Utilization: " << util_pct << "%\n";
        
        cout << "\nPer Allocation Fragmentation:\n";
        sys_buddy->dump_allocations();
    } else {
        cout << "No allocator active\n";
    }

    // 2. VIRTUAL MEMORY STATS
    cout << "\n----- Virtual Memory -----\n";
    int h = get_page_hits();
    int f = get_page_faults();
    int total_vm = h + f;
    
    cout << "Total Page Hits: " << h << "\n";
    cout << "Total Page Faults: " << f << "\n";
    
    if (total_vm > 0)
        cout << "Fault Rate: " << (f * 100.0 / total_vm) << "%\n";
    else
        cout << "Fault Rate: 0%\n";
    
    cout << "\nPer-Process Frame Usage:\n";
    if (!any_vm_initialized()) {
        cout << "No virtual memory initialized for any process.\n";
    } else {
        for (int pid : get_initialized_pids()) {
            int used = get_used_frames(pid);
            if (used == 0)
                cout << "PID " << pid << ": NIL\n";
            else
                cout << "PID " << pid << ": " << used << "/" << get_total_frames() << " frames used\n";
        }
    }

    // 3. CACHE STATS
    cout << "\n----- Cache -----\n";
    primary_cache->print_stats("L1");
    secondary_cache->print_stats("L2");
    cout << "Total Memory Access Cycles: " << total_cycles << "\n";
    cout << "Disk Penalty per fault: " << disk_penalty << "\n";
}

// ==========================================
//    MAIN ENTRY POINT
// ==========================================

int main() {
    int user_selection;

    while (true) {
        show_main_menu();
        
        if (!(cin >> user_selection)) {
            flush_input();
            continue;
        }

        switch (user_selection) {
            // CASE 1: CONFIGURATION 
            case 1: {
                int input_ram, input_page;
                cout << "\n>> Configure RAM Size (bytes): ";
                cin >> input_ram;
                cout << ">> Configure Page Size (bytes): ";
                cin >> input_page;

                if (input_ram <= 0 || input_page <= 0) {
                    cout << "Error: Values must be positive integers.\n";
                    break;
                }
                if (input_ram % input_page != 0) {
                    cout << "Error: RAM must be a multiple of Page Size.\n";
                    break;
                }

                config_ram_size = input_ram;
                config_page_size = input_page;
                system_frame_count = input_ram / input_page;

                reset_allocation_stats();
                init_memory(config_ram_size);
                reset_vm_system(config_ram_size, config_page_size);
                buddy_ids.clear();
                
                delete sys_buddy;
                delete primary_cache;
                delete secondary_cache;

                sys_buddy = new BuddyAllocator(config_ram_size, 128);

                int s1, bl1, as1; 
                int s2, bl2, as2; 
                
                cout << "\n>> L1 Cache (Size BlockSize Assoc): ";
                cin >> s1 >> bl1 >> as1;

                cout << ">> L2 Cache (Size BlockSize Assoc): ";
                cin >> s2 >> bl2 >> as2;

                primary_cache = new Cache(s1, bl1, as1);
                secondary_cache = new Cache(s2, bl2, as2);

                current_strategy = STRAT_UNSET;
                total_cycles = 0;
                
                // Added Detailed Echo matching Friend's Output
                cout << "\n[System] Initialization Complete.\n";
                cout << "Physical Memory : " << config_ram_size << " bytes\n";
                cout << "Page Size (for Virtual Memory simulations)  : " << config_page_size << " bytes\n";
                cout << "Total Frames (for Virtual Memory simulations)  : " << system_frame_count << "\n";
                cout << "L1 Size: "<< s1 << "B | Block Size:"<< bl1 << "B | Assoc: " << as1 << "-way\n";
                cout << "L2 Size: "<< s2 << "B | Block Size:"<< bl2 << "B | Assoc: " << as2 << "-way\n";
                break;
            }

            // CASE 2: ALLOCATION 
            case 2: {
                if (config_ram_size == 0) {
                    cout << "Alert: Initialize system first.\n";
                    break;
                }

                int op_choice;
                cout << "\n   [1] Allocate (Malloc)\n   [2] Deallocate (Free)\n   Choice: ";
                cin >> op_choice;

                if (op_choice == 1) {
                    int bytes_needed;
                    cout << "   Bytes required: ";
                    cin >> bytes_needed;

                    if (current_strategy == STRAT_UNSET) {
                        int mode_sel;
                        cout << "   Select Mode: [1] Linear (FF/BF/WF) [2] Buddy System: ";
                        cin >> mode_sel;
                        current_strategy = (mode_sel == 2) ? STRAT_BUDDY : STRAT_LINEAR;
                        
                        // Echo selection to match friend's behavior
                        cout << "Allocator mode set to " << (current_strategy == STRAT_BUDDY ? "BUDDY\n" : "LINEAR\n");
                    }

                    int result_addr = -1;

                    if (current_strategy == STRAT_BUDDY) {
                        result_addr = sys_buddy->buddy_malloc(bytes_needed);
                    } else {
                        int lin_algo;
                        cout << "   Algo: [1] FirstFit [2] BestFit [3] WorstFit: ";
                        cin >> lin_algo;
                        
                        if (lin_algo == 1) result_addr = malloc_first_fit(bytes_needed);
                        else if (lin_algo == 2) result_addr = malloc_best_fit(bytes_needed);
                        else if (lin_algo == 3) result_addr = malloc_worst_fit(bytes_needed);
                    }

                    if (result_addr == -1) {
                        cout << "Allocation failed\n"; // Matches friend's string
                    } else {
                        int blk_id = (current_strategy == STRAT_BUDDY) 
                                     ? buddy_ids[result_addr] 
                                     : get_block_id(result_addr);
                        
                        // Matches friend's allocation success string format
                        cout << "Allocated block id=" << blk_id << " at address=0x" << hex << result_addr << dec << "\n";
                    }

                } else if (op_choice == 2) {
                    int target_id;
                    cout << "   Block ID to free: ";
                    cin >> target_id;

                    int addr_to_free = get_block_start_by_id(target_id);
                    
                    if (addr_to_free == -1) {
                        for (auto &pair : buddy_ids) {
                            if (pair.second == target_id) {
                                addr_to_free = pair.first;
                                break;
                            }
                        }
                    }

                    if (addr_to_free != -1) {
                        if (buddy_ids.count(addr_to_free)) sys_buddy->buddy_free(addr_to_free);
                        else free_block(addr_to_free);
                        cout << "Block " << target_id << " freed\n"; // Matches friend's string
                    } else {
                        cout << "No block with id=" << target_id << "\n"; // Matches friend's string
                    }
                }
                break;
            }

            // CASE 3: VIRTUAL MEMORY
            case 3: {
                if (config_ram_size == 0) {
                    cout << "Alert: Initialize system first.\n";
                    break;
                }
                bool stay_in_vm = true;
                while(stay_in_vm) {
                    show_vm_submenu();
                    int vm_act;
                    cin >> vm_act;

                    switch(vm_act) {
                        case 1: { // vm_init
                            int p, v;
                            cout << "   PID: "; cin >> p;
                            cout << "   Size: "; cin >> v;
                            init_vm(p, v);
                            break;
                        }
                        case 2: { // access
                            int p, vaddr;
                            cout << "   PID: "; cin >> p;
                            cout << "   Virtual Addr: "; cin >> vaddr;
                            int phys = vm_access(p, vaddr);
                            cout << "Physical address = " << phys << "\n"; // Matches friend's string
                            if (phys != -1) perform_memory_lookup(phys);
                            break;
                        }
                        case 3: { // vm_table
                            int p;
                            cout << "   PID: "; cin >> p;
                            dump_page_table(p);
                            break;
                        }
                        case 4: 
                            stay_in_vm = false; 
                            break;
                        default:
                            cout << "Invalid VM choice.\n";
                    }
                }
                break;
            }

            // CASE 4: STATISTICS & DUMP
            case 4: {
                int stat_choice;
                cout << "\n   [1] Visual Map (Dump)\n   [2] Detailed Stats\n   Choice: ";
                cin >> stat_choice;
                
                if (stat_choice == 1) {
                    if (current_strategy == STRAT_BUDDY && sys_buddy) {
                        cout << " Buddy allocator in use \n";
                        sys_buddy->dump_allocations();
                        sys_buddy->dump_free_lists();
                    } else {
                        cout << "Linear allocator in use \n";
                        dump_memory();
                    }
                } else {
                    print_system_report();
                }
                break;
            }

            // CASE 5: EXIT
            case 5:
                cout << "Shutting down simulator.\n";
                return 0;

            default:
                cout << "Unknown selection.\n";
        }
    }
}