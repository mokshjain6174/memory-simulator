# Memory Management Simulator

## Overview

This project is a comprehensive simulation of an **Operating System Memory Management Unit (MMU)**.  
It models the interaction between:

- **Physical Memory Allocation**
- **Virtual Memory (Paging)**
- **Multi-Level Cache Hierarchy**

The goal of this simulator is to visualize and analyze the performance trade-offs of different allocation strategies (**First Fit, Best Fit, Worst Fit, Buddy System**) and the impact of paging and caching on overall system performance.

---

**Demo Video** : [https://drive.google.com/file/d/15tbWFuZFEO5mgyFKbxJ41Sa27bM2g8Qh/view?usp=sharing](https://drive.google.com/file/d/15tbWFuZFEO5mgyFKbxJ41Sa27bM2g8Qh/view?usp=sharing)

---
##  Project Structure
MemorySimulator/
├── include/                 # Header files
│   ├── buddy.h
│   ├── cache.h
│   ├── memory.h
│   └── vm.h
├── src/                     # Source code
│   ├── buddy.cpp
│   ├── cache.cpp
│   ├── main.cpp             # Entry point (Menu-Driven Interface)
│   ├── memory.cpp
│   └── vm.cpp
├── test/                    # Input test files
│   ├── buddy.txt
│   ├── cache.txt
│   ├── linear.txt
│   └── vm.txt
├── output/                  # Output logs (generated from tests)
│   ├── all_tests.txt
│   ├── buddy_out.txt
│   ├── cache_out.txt
│   ├── linear_out.txt
│   └── vm_out.txt
├── Makefile                 # Compilation script
├── Readme.md                # Project documentation
├── run_all_tests.bat        # Batch script for running tests (Windows)
├── run_all_tests.sh         # Shell script for running tests (Linux/Mac)
├── .gitignore
└── memsim.exe               # Compiled executable


---
## ✨ Features

### 1. Dynamic Memory Allocation
- **Linear Allocation Strategies:** First Fit, Best Fit, and Worst Fit
- **Buddy System:** Power-of-two allocator with recursive splitting and coalescing
- **Fragmentation Analysis:** Tracks internal and external fragmentation in real time

### 2. Virtual Memory System
* **Paging:** Simulates address translation from Virtual Addresses (VA) to Physical Addresses (PA) using per-process page tables.
* **Page Fault Handling:** Simulates disk access latency and frame allocation upon page faults.
* **Page Replacement:** Implements an **LRU (Least Recently Used)** eviction policy to manage limited physical frames.

### 3. Cache Simulation
* **Multi-Level Hierarchy:** Simulates **L1** (Primary) and **L2** (Secondary) caches.
* **Configurable Architecture:** Supports custom cache sizes, block sizes, and set associativity.
* **Replacement Policy:** Implements a **FIFO (First-In, First-Out)** strategy for cache line eviction.

## ⚙️ Prerequisites

- **C++ Compiler:** GCC / MinGW (`g++`) with **C++17** support
- **Make:** Optional 

---

## 🚀 How to Build and Run

To compile and run the project, try the following methods in order.
---
### Method 1: Using `make`
Open your terminal in the project directory and run:

```bash
make
./memsim
```
---
### Method 2: Using `mingw32-make` (recommended **MSYS MINGW64**)
If the standard make command is not found, try using the MinGW specific make command:

```bash
mingw32-make
./memsim
```
---
### Method 3: Manual Compilation (Terminal)
If you do not have Make installed, you can compile the source files directly using g++:

```bash
g++ -std=c++17 -O2 -Wall src/main.cpp src/memory.cpp src/buddy.cpp src/cache.cpp src/vm.cpp -Iinclude -o memsim
./memsim
```
---
### Method 4: Windows Command Prompt (CMD)
If the above methods fail or you prefer using the standard Windows Command Prompt:

1. Open Command Prompt (cmd).
2. Navigate to your project directory:
```
cd /d E:\MemorySimulator
```
3. Run the compilation command manually:
```
g++ -std=c++17 -O2 -Wall src/main.cpp src/memory.cpp src/buddy.cpp src/cache.cpp src/vm.cpp -Iinclude -o memsim
```
4. Run the executable:
```
memsim.exe
```
---


## 🚀 How to Run Tests
---
### For Linux, MacOS and MSYS2 / Git Bash (Windows)
### Manually
Example - It's output stored in buddy_out.txt
```
./memsim.exe < test/buddy.txt  > output/buddy_out.txt
```
### Automated
Running each test and saving all outputs in in one file `all_tests.txt`
```
chmod +x run_all_tests.sh
./run_all_tests.sh
```

---
### For Windows (cmd)
### Manually
Example - It's output is stored in buddy_out.txt
```
memsim.exe < test\buddy.txt > output\buddy_out.txt
```
### Automated
Running each test and saving all outputs in in one file `all_tests.txt`
```
run_all_tests.bat
```
---

## Assumptions & Simplifications
* Implicit demand paging: unmapped pages trigger a page fault and are automatically mapped (no segmentation faults).
* Heap & paging are independent: allocators manage heap; paging manages frames/page tables separately.
* No protection bits: R/W/X permissions are not simulated.
* Abstracted CPU behavior: we model translation flow, not full instruction execution or traps.
* Simplified replacement: LRU for pages, FIFO for cache (no dirty‑bit/disk writes).

---



