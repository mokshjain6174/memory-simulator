#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <algorithm>
#include <string>
#include <cstdint>


const int ADDR_BITS = 32;

enum ReplacementPolicy { FIFO, LRU, LFU };

struct CacheConfig {
    std::string name;
    size_t size;          // Total size in bytes
    size_t blockSize;     // Block size in bytes
    size_t associativity; // Ways (1 = Direct Mapped)
    int accessTime;       // Cycles to check this cache
    ReplacementPolicy policy; // FIFO, LRU, or LFU
};

struct CacheLine {
    bool valid = false;
    uint64_t tag = 0;
    
    // Metadata for Replacement Policies
    uint64_t insertionTime = 0; // For FIFO
    uint64_t lastAccessTime = 0; // For LRU
    int frequency = 0;           // For LFU
};

class CacheLevel {
private:
    CacheConfig config;
    size_t numSets;
    size_t indexBits;
    size_t offsetBits;
    size_t tagBits;

    std::vector<std::vector<CacheLine>> sets;
    
    // Stats
    long long hits = 0;
    long long misses = 0;
    
    // Global counter to simulate "Time"
    uint64_t globalCounter = 0;

    CacheLevel* nextLevel = nullptr;

public:
    CacheLevel(CacheConfig cfg, CacheLevel* next = nullptr) : config(cfg), nextLevel(next) {
        size_t numBlocks = config.size / config.blockSize;
        numSets = numBlocks / config.associativity;

        offsetBits = std::log2(config.blockSize);
        indexBits = std::log2(numSets);
        tagBits = ADDR_BITS - indexBits - offsetBits;

        sets.resize(numSets, std::vector<CacheLine>(config.associativity));
    }

    int access(uint64_t address) {
        globalCounter++; //Increment time on every access

        //Parse Address
        uint64_t offsetMask = (1ULL << offsetBits) - 1; //1ULL ensures 64-bit shift
        uint64_t indexMask = ((1ULL << indexBits) - 1);
        
        uint64_t setIndex = (address >> offsetBits) & indexMask;
        uint64_t tag = (address >> (offsetBits + indexBits));

        //Search for Tag
        for (auto& line : sets[setIndex]) {
            if (line.valid && line.tag == tag) {
                // --- HIT ---
                hits++;
                
                // Update Metadata on HIT
                if (config.policy == LRU) {
                    line.lastAccessTime = globalCounter;
                } else if (config.policy == LFU) {
                    line.frequency++;
                    line.lastAccessTime = globalCounter; // Update for tie-breaking
                }
                // FIFO does NOT update anything on hit

                return config.accessTime;
            }
        }

        //--- MISS ---
        misses++;
        
        //Fetch from lower level
        int missPenalty = 0;
        if (nextLevel) {
            missPenalty = nextLevel->access(address);
        } else {
            missPenalty = 100; //Main Memory delay
        }

        //Install new block
        install(setIndex, tag);

        return config.accessTime + missPenalty;
    }

    void install(uint64_t setIndex, uint64_t tag) {
        auto& set = sets[setIndex];
        
        //Look for Empty Slot first
        for (auto& line : set) {
            if (!line.valid) {
                line.valid = true;
                line.tag = tag;
                line.insertionTime = globalCounter;
                line.lastAccessTime = globalCounter;
                line.frequency = 1;
                return;
            }
        }

        //If Full, Find Victim based on Policy
        int victimIndex = 0;
        
        if (config.policy == FIFO) {
            //Evict oldest insertion time
            uint64_t minTime = set[0].insertionTime;
            for (size_t i = 1; i < set.size(); ++i) {
                if (set[i].insertionTime < minTime) {
                    minTime = set[i].insertionTime;
                    victimIndex = i;
                }
            }
        } 
        else if (config.policy == LRU) {
            //Evict oldest last access time
            uint64_t minTime = set[0].lastAccessTime;
            for (size_t i = 1; i < set.size(); ++i) {
                if (set[i].lastAccessTime < minTime) {
                    minTime = set[i].lastAccessTime;
                    victimIndex = i;
                }
            }
        } 
        else if (config.policy == LFU) {
            //Evict smallest frequency
            int minFreq = set[0].frequency;
            for (size_t i = 1; i < set.size(); ++i) {
                if (set[i].frequency < minFreq) {
                    minFreq = set[i].frequency;
                    victimIndex = i;
                } 
                //If frequencies are equal, use LRU (oldest access)
                else if (set[i].frequency == minFreq) {
                    if (set[i].lastAccessTime < set[victimIndex].lastAccessTime) {
                        victimIndex = i;
                    }
                }
            }
        }

        // Replace Victim
        set[victimIndex].tag = tag;
        set[victimIndex].insertionTime = globalCounter;
        set[victimIndex].lastAccessTime = globalCounter;
        set[victimIndex].frequency = 1; // Reset frequency for new block
    }

    void printStats() {
        double total = hits + misses;
        double ratio = (total > 0) ? (double(hits) / total) * 100.0 : 0.0;
        
        std::cout << "--- " << config.name << " Stats ---" << std::endl;
        std::cout << "Policy: " << (config.policy == FIFO ? "FIFO" : (config.policy == LRU ? "LRU" : "LFU")) << std::endl;
        std::cout << "Size: " << config.size << "B | Ways: " << config.associativity << std::endl;
        std::cout << "Hits: " << hits << " | Misses: " << misses << std::endl;
        std::cout << "Hit Ratio: " << std::fixed << std::setprecision(2) << ratio << "%" << std::endl;
        std::cout << "-----------------------" << std::endl;
    }
};

int main() {
    std::cout << "=== Multilevel Cache Simulation ===" << std::endl;

    // --- CONFIGURATION GUIDE ---
    // To change the Cache Replacement Policy, modify the last parameter in CacheConfig.
    // Options:
    //   FIFO  -> First-In, First-Out (Evicts oldest inserted block)
    //   LRU   -> Least Recently Used (Evicts block unused for longest time)
    //   LFU   -> Least Frequently Used (Evicts block with fewest hits)
    
    // Example: To use FIFO, change 'LRU' to 'FIFO' below:
    CacheConfig l2Config = {"L2 Cache", 4096, 64, 4, 10, FIFO}; 
    CacheLevel* L2 = new CacheLevel(l2Config, nullptr);
    CacheConfig l1Config = {"L1 Cache", 1024, 64, 2, 1, FIFO}; 
    CacheLevel* L1 = new CacheLevel(l1Config, L2);

    // --- TEST TRACE ---
    std::vector<uint64_t> trace = {
        0x1000, 0x1040, 0x1080, 0x10C0, // Fill a set
        0x1000, // Hit
        0x2000, // New block
        0x1040, // Access older block
        0x1080,
        0x3000,
        0x1000
    };

    int totalCycles = 0;
    for (uint64_t addr : trace) {
        totalCycles += L1->access(addr);
    }
    std::cout << "\n=== Results ===" << std::endl;
    L1->printStats();
    L2->printStats();
    std::cout << "Total AMAT: " << (double)totalCycles / trace.size() << " cycles\n";

    delete L1;
    delete L2;
    return 0;
}