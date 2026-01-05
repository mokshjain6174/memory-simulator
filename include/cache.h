#ifndef CACHE_H
#define CACHE_H

#include <vector>
#include <string>

struct CacheConfig {
    std::string name;
    int size;
    int blockSize;
    int associativity;
    int accessTime;
};

struct CacheLine {
    bool valid = false;
    int tag = -1;
    int fifo_counter = 0;
};

class CacheLevel {
private:
    CacheConfig config;
    int numSets;
    std::vector<std::vector<CacheLine>> sets;
    int hits = 0;
    int misses = 0;
    int global_counter = 0;
    CacheLevel* nextLevel;

public:
    CacheLevel(CacheConfig cfg, CacheLevel* next = nullptr);
    int access(int address); 
    void printStats();
};

#endif