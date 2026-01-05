#include <iostream>
#include <string>
#include <vector>
#include <iomanip>

#include "include/memory.h"
#include "include/buddy.h"
#include "include/cache.h"
#include "include/vm.h"

using namespace std;

MemoryManager* linearMem = nullptr;
BuddyAllocator* buddyMem = nullptr;
VirtualMemory* virtMem = nullptr;
CacheLevel* l1 = nullptr;
CacheLevel* l2 = nullptr;

enum AllocatorMode { NONE, LINEAR, BUDDY };
AllocatorMode current_mode = NONE;

int PHYS_MEM_SIZE = 0;
int PAGE_SIZE = 0;

const int L1_LATENCY = 1;
const int L2_LATENCY = 5;
const int MEM_PENALTY = 50;

//Simulate Cache Access
void simulate_cache_access(int paddr) {
    if (!l1 || !l2) return;

    // The access() function returns the TOTAL cycles spent
    int total_time = l1->access(paddr); 
    
    if (total_time == L1_LATENCY) {
        // Time == 1
        cout << "L1 hit!\n";
    } 
    else if (total_time <= (L1_LATENCY + L2_LATENCY)) {
        // Time == 1 + 5 = 6
        cout << "L1 miss. L2 hit.\n";
    } 
    else {
        // Time > 6 (Must have gone to RAM)
        cout << "L1 miss. L2 miss. Accessing main memory.\n";
    }
}

void print_help() {
    cout << "\nCommands:\n";
    cout << "   init <mem_size> <page_size>   : Setup system.\n";
    cout << "   alloc ff <bytes>              : Alloc (First Fit)\n";
    cout << "   alloc bf <bytes>              : Alloc (Best Fit)\n";
    cout << "   alloc buddy <bytes>           : Alloc (Buddy)\n";
    cout << "   free <address>                : Free block\n";
    cout << "   vm_init <pid> <size>          : Create process\n";
    cout << "   access <pid> <vaddr>          : Access Memory\n";
    cout << "   stats                         : Show stats\n";
    cout << "   exit                          : Quit\n";
}

int main() {
    string cmd;
    cout << "=== MEMORY SIMULATOR ===\nType 'help' for commands.\n";

    while (true) {
        cout << ">> ";
        if (!(cin >> cmd)) break;

        if (cmd == "exit") break;
        if (cmd == "help") { print_help(); continue; }

        if (cmd == "init") {
            int memSize, pageSize;
            if (!(cin >> memSize >> pageSize)) break;

            int c1, b1, a1, c2, b2, a2;
            cout << "Enter L1 Config [Size BlockSize Assoc]: ";
            cin >> c1 >> b1 >> a1;
            cout << "Enter L2 Config [Size BlockSize Assoc]: ";
            cin >> c2 >> b2 >> a2;

            delete linearMem; delete buddyMem; delete virtMem; delete l1; delete l2;
            
            PHYS_MEM_SIZE = memSize;
            PAGE_SIZE = pageSize;

            linearMem = new MemoryManager(memSize);
            buddyMem = new BuddyAllocator(memSize);
            virtMem = new VirtualMemory(memSize, pageSize);
            
            CacheConfig l2Config = {"L2", c2, b2, a2, L2_LATENCY};
            l2 = new CacheLevel(l2Config, nullptr); 

            CacheConfig l1Config = {"L1", c1, b1, a1, L1_LATENCY};
            l1 = new CacheLevel(l1Config, l2);
            
            current_mode = NONE;
            cout << "System Initialized.\n";
        }
        else if (cmd == "alloc") {
            string type; int size;
            cin >> type >> size;
            
            if (PHYS_MEM_SIZE == 0) { cout << "Run init first.\n"; continue; }

            int addr = -1;
            if (type == "buddy") {
                current_mode = BUDDY;
                addr = buddyMem->buddy_malloc(size);
            } else {
                current_mode = LINEAR;
                if (type == "bf") linearMem->set_strategy(BEST_FIT);
                else if (type == "wf") linearMem->set_strategy(WORST_FIT);
                else linearMem->set_strategy(FIRST_FIT);
                addr = linearMem->my_malloc(size);
            }

            if (addr != -1) cout << "Allocated at " << addr << "\n";
            else cout << "Allocation Failed\n";
        }
        else if (cmd == "free") {
            int addr; cin >> addr;
            if (current_mode == BUDDY && buddyMem) buddyMem->buddy_free(addr);
            else if (linearMem) linearMem->my_free(addr);
        }
        else if (cmd == "access") {
            int pid, vaddr; cin >> pid >> vaddr;
            if (virtMem && l1) {
                int paddr = virtMem->access(pid, vaddr);
                if (paddr != -1) simulate_cache_access(paddr);
            }
        }
        else if (cmd == "stats") {
            if (linearMem) linearMem->dump_memory();
            if (buddyMem) buddyMem->dump_buddy_memory();
            if (virtMem) virtMem->print_stats();
            if (l1) { l1->printStats(); l2->printStats(); }
        }
    }
    return 0;
}