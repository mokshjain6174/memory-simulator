# Memory Management Simulator

## Overview

This project is a comprehensive simulation of an **Operating System Memory Management Unit (MMU)**.  
It models the interaction between:

- **Physical Memory Allocation**
- **Virtual Memory (Paging)**
- **Multi-Level Cache Hierarchy**

The goal of this simulator is to visualize and analyze the performance trade-offs of different allocation strategies (**First Fit, Best Fit, Worst Fit, Buddy System**) and the impact of paging and caching on overall system performance.

---

## ✨ Features

### 1. Dynamic Memory Allocation
- **Linear Allocation Strategies:** First Fit, Best Fit, and Worst Fit
- **Buddy System:** Power-of-two allocator with recursive splitting and coalescing
- **Fragmentation Analysis:** Tracks internal and external fragmentation in real time

---

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
- **Make:** Optional (recommended via **MSYS2 MinGW64**)

---

## 🚀 How to Build and Run

To compile and run the project, try the following methods in order.

### Method 1: Using `make`
Open your terminal in the project directory and run:

```bash
make
./memsim
```
### Method 2: Using mingw32-make
If the standard make command is not found, try using the MinGW specific make command:

```bash
mingw32-make
./memsim
```

### Method 3: Manual Compilation (Terminal)
If you do not have Make installed, you can compile the source files directly using g++:

```bash
g++ -std=c++17 -O2 -Wall src/main.cpp src/memory.cpp src/buddy.cpp src/cache.cpp src/vm.cpp -Iinclude -o memsim
./memsim
```

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
