#include "../include/cache.h"
#include <iostream>
#include <cmath>
#include <iomanip>

CacheLevel::CacheLevel(CacheConfig cfg, CacheLevel* next) : config(cfg), nextLevel(next) {
    int numBlocks = config.size / config.blockSize;
    numSets = numBlocks / config.associativity;
    sets.resize(numSets, std::vector<CacheLine>(config.associativity));
}

int CacheLevel::access(int address) {
    int block_addr = address / config.blockSize;
    int setIndex = block_addr % numSets;
    int tag = block_addr / numSets;

    // Check Hit
    for (auto& line : sets[setIndex]) {
        if (line.valid && line.tag == tag) {
            hits++;
            return config.accessTime;
        }
    }

    // Miss
    misses++;
    int penalty = (nextLevel) ? nextLevel->access(address) : 100;

    // Install
    auto& set = sets[setIndex];
    int replaceIdx = -1;
    int minCounter = 2147483647;

    // Find invalid or oldest
    for (int i = 0; i < set.size(); i++) {
        if (!set[i].valid) {
            replaceIdx = i;
            break;
        }
        if (set[i].fifo_counter < minCounter) {
            minCounter = set[i].fifo_counter;
            replaceIdx = i;
        }
    }

    set[replaceIdx].valid = true;
    set[replaceIdx].tag = tag;
    set[replaceIdx].fifo_counter = ++global_counter;

    return config.accessTime + penalty;
}

void CacheLevel::printStats() {
    std::cout << "Cache " << config.name << " -> Hits: " << hits << " | Misses: " << misses << std::endl;
}