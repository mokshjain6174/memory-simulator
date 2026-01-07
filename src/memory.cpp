#include <list>
#include <vector>
#include <iostream>
#include <iomanip>
#include "../include/memory.h"
#include "../include/buddy.h"
#include <sstream>
using namespace std;

int total_alloc_requests = 0;
int successful_allocs = 0;
int failed_allocs = 0;
int total_memory_size = 0;

int next_block_id = 1;

unordered_map<int,int> buddy_ids;
list<Block> memory_blocks;

void init_memory(int total_size) {
    memory_blocks.clear();
    total_memory_size = total_size;
    next_block_id = 1;

    Block initial;
    initial.start = 0;
    initial.size = total_size;
    initial.free = true;
    initial.id = -1;

    memory_blocks.push_back(initial);
}

void reset_allocation_stats() {
    total_alloc_requests = 0;
    successful_allocs  = 0;
    failed_allocs    = 0;
}

void dump_memory() {
    cout << "----- Memory Dump -----\n";

    for (auto &b : memory_blocks) {
        int end = b.start + b.size - 1;

        cout << "[0x"
             << hex << setw(4) << setfill('0') << b.start
             << " - 0x"
             << hex << setw(4) << setfill('0') << end
             << dec << "] ";

        if (b.free)
            cout << "FREE\n";
        else
            cout << "USED (id=" << b.id << ")\n";
    }

    cout << "-----------------------\n";
}

int malloc_first_fit(int size) {
    total_alloc_requests++;
    if (size <= 0) {
        cout << "Invalid allocation size\n";
        failed_allocs++;
        return -1;
    }

    for (auto it = memory_blocks.begin(); it != memory_blocks.end(); ++it) {

        if (it->free && it->size >= size) {

            int alloc_start = it->start;

            if (it->size == size) {
                it->free = false;
                it->id = next_block_id++;
            } else {
                Block allocated{it->start, size, false, next_block_id++};
                Block remaining{it->start + size,
                                it->size - size,
                                true,
                                -1};

                it = memory_blocks.erase(it);
                it = memory_blocks.insert(it, allocated);
                ++it;
                memory_blocks.insert(it, remaining);
            }

            successful_allocs++;
            return alloc_start;
        }
    }

    failed_allocs++;
    return -1;
}

void free_block(int start_address) {
    for (auto it = memory_blocks.begin(); it != memory_blocks.end(); ++it) {

        if (it->start == start_address && !it->free) {

            it->free = true;
            it->id = -1;

            if (it != memory_blocks.begin()) {
                auto prev = it;
                --prev;
                if (prev->free) {
                    prev->size += it->size;
                    it = memory_blocks.erase(it);
                    it = prev;
                }
            }

            auto next = it;
            ++next;
            if (next != memory_blocks.end() && next->free) {
                it->size += next->size;
                memory_blocks.erase(next);
            }

            return;
        }
    }
}

int malloc_best_fit(int size) {
    total_alloc_requests++;
    if (size <= 0) {
        cout << "Invalid allocation size\n";
        failed_allocs++;
        return -1;
    }

    auto best = memory_blocks.end();

    for (auto it = memory_blocks.begin(); it != memory_blocks.end(); ++it) {
        if (it->free && it->size >= size) {
            if (best == memory_blocks.end() || it->size < best->size)
                best = it;
        }
    }

    if (best == memory_blocks.end()) {
        failed_allocs++;
        return -1;
    }

    int alloc_start = best->start;

    if (best->size == size) {
        best->free = false;
        best->id = next_block_id++;
    } else {
        Block allocated{best->start, size, false, next_block_id++};
        Block remaining{best->start + size,
                        best->size - size,
                        true,
                        -1};

        best = memory_blocks.erase(best);
        best = memory_blocks.insert(best, allocated);
        ++best;
        memory_blocks.insert(best, remaining);
    }

    successful_allocs++;
    return alloc_start;
}

int malloc_worst_fit(int size) {
    total_alloc_requests++;
    if (size <= 0) {
        cout << "Invalid allocation size\n";
        failed_allocs++;
        return -1;
    }

    auto worst = memory_blocks.end();

    for (auto it = memory_blocks.begin(); it != memory_blocks.end(); ++it) {
        if (it->free && it->size >= size) {
            if (worst == memory_blocks.end() || it->size > worst->size)
                worst = it;
        }
    }

    if (worst == memory_blocks.end()) {
        failed_allocs++;
        return -1;
    }

    int alloc_start = worst->start;

    if (worst->size == size) {
        worst->free = false;
        worst->id = next_block_id++;
    } else {
        Block allocated{worst->start, size, false, next_block_id++};
        Block remaining{worst->start + size,
                        worst->size - size,
                        true,
                        -1};

        worst = memory_blocks.erase(worst);
        worst = memory_blocks.insert(worst, allocated);
        ++worst;
        memory_blocks.insert(worst, remaining);
    }

    successful_allocs++;
    return alloc_start;
}

int internal_fragmentation() {
    return 0;  
}

int external_fragmentation() {
    int total_free = 0;
    int max_free = 0;

    for (auto &b : memory_blocks) {
        if (b.free) {
            total_free += b.size;
            max_free = max(max_free, b.size);
        }
    }

    return total_free - max_free;
}

double memory_utilization() {
    int used = 0, total = 0;

    for (auto &b : memory_blocks) {
        total += b.size;
        if (!b.free) used += b.size;
    }
    
    return total ? (used * 100.0 / total) : 0.0;
}

void allocation_stats() {
    cout << "Allocation Requests: " << total_alloc_requests << "\n";
    cout << "Successful Allocations: " << successful_allocs << "\n";
    cout << "Failed Allocations: " << failed_allocs << "\n";

    double rate = total_alloc_requests
        ? (successful_allocs * 100.0 / total_alloc_requests)
        : 0.0;

    cout << "Allocation Success Rate: " << rate << "%\n";
}

int get_block_id(int start_address) {
    for (auto &b : memory_blocks) {
        if (!b.free && b.start == start_address)
            return b.id;
    }
    return -1;
}

int get_block_start_by_id(int id) {
    for (auto &b : memory_blocks) {
        if (!b.free && b.id == id)
            return b.start;
    }
    return -1;
}