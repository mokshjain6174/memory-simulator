# Design Document: Memory Management Simulator

**Project:** Operating System Memory Management Simulator

---

## 1. Introduction
This document details the design and implementation of a Memory Management Simulator. The system models the interaction between the **CPU**, **Memory Management Unit (MMU)**, **Cache Hierarchy**, and **Physical RAM**. The goal is to simulate how an Operating System manages memory resources, handles virtual-to-physical translation, and optimizes performance using caches.

---

## 2. Memory Layout and Assumptions

### 2.1 Physical Memory (RAM)
* **Structure:** Modeled as a contiguous array of bytes.
* **Frames:** Physical memory is logically divided into fixed-size chunks called **Frames** (default 16 Bytes).
* **Global Tracking:** The system uses a global **Frame Table** to track which Process ID (PID) owns each frame. If a frame is empty, it is marked as free (`-1`).

### 2.2 Virtual Memory (The Process View)
* **Structure:** Every process operates under the illusion of having access to a large, continuous block of memory (Virtual Address Space).
* **Pages:** Virtual memory is divided into **Pages**, which are the same size as physical Frames.
* **Isolation:** Process A cannot access Process B's memory because their Virtual Pages map to different Physical Frames.

### 2.3 System Assumptions
1.  **Single-Threaded Simulation:** The simulator runs as a single process. Concurrency is modeled logically, not via actual OS threads.
2.  **Simulated Disk I/O:** Reading from "Disk" (during a Page Fault) is simulated by adding a time penalty (cycle count), not by reading actual files on the hard drive.
3.  **Volatile Storage:** All memory states are lost when the simulator exits.
  
---

## 3. Allocation Strategy Implementations

The simulator allows the user to switch between two fundamentally different ways of managing the "Heap" (dynamic memory).

### 3.1 Linear Allocator
This strategy manages memory as a linked list of blocks. When a request comes in, it scans the list to find a suitable hole.

* **First Fit:**
    * *Logic:* Scans from the beginning and picks the **first** free block that is big enough.
    * *Pros/Cons:* Very fast allocation, but tends to leave "splinters" (small gaps) at the beginning of memory.
* **Best Fit:**
    * *Logic:* Scans the **entire** list to find the **smallest** free block that fits the request perfectly (or closely).
    * *Pros/Cons:* Minimizes wasted space by saving large holes for later, but is slower because it searches the whole list.
* **Worst Fit:**
    * *Logic:* Deliberately picks the **largest** available free block.
    * *Pros/Cons:* Ensures the leftover gap is large enough to be potentially useful, but quickly consumes big contiguous blocks.

### 3.2 Buddy System Design
The Buddy System is designed to minimize external fragmentation and allow fast merging (coalescing).

* **Powers of Two:** Memory is strictly divided into blocks of size $2^k$ (e.g., 16, 32, 64, 128 bytes).
* **Splitting:** If a user requests 20 bytes, the system rounds up to 32 bytes. If it only has a 64-byte block, it splits it into two 32-byte "Buddies." One is used; the other remains free.
* **Coalescing (Merging):** When a block is freed, the system checks its "Buddy" (its neighbor in memory). If the Buddy is also free, they merge back into a larger block. This happens recursively up the chain.

**Diagram: Buddy Splitting (Requesting 20 bytes from 128B RAM)**
```text
[ 128 Byte Block ]  <-- Initial State
      |
      V (Split)
[ 64 Byte ] [ 64 Byte ]
      |
      V (Split left child)
[ 32 Byte ] [ 32 Byte ] [ 64 Byte ]
    ^
  (Allocated) (Free)      (Free)
```

---

## 4. Virtual Memory Model & Address Translation

### 4.1 The Page Table
Every process has a **Page Table** that acts as a map. It translates "Virtual Page Numbers" (VPN) to "Physical Frame Numbers" (PFN).

* **Valid Bit:** Indicates if the page is currently loaded in RAM (1) or if it is on Disk (0).
* **Last Access Time:** Stores the timestamp of the last read/write. This is used by the replacement policy.

### 4.2 Address Translation Flow
When the CPU requests a Virtual Address, the following sequence occurs:

1.  **Calculate Page Number:** `VPN = Virtual_Address / Page_Size`.
2.  **Lookup:** The system checks the Page Table for this VPN.
3.  **Hit:** If the page is valid, the system retrieves the Frame Number.
    * *Physical Address = (Frame_Number * Page_Size) + Offset*.
4.  **Miss (Page Fault):**
    * The page is NOT in RAM.
    * **Simulate Disk Load:** A penalty of 200 cycles is added to the system timer.
    * **Find Victim:** If RAM is full, the replacement policy (LRU) runs to kick out an old page.
    * **Update Map:** The new page is loaded into the frame and the Page Table is updated.

**Diagram: Address Translation Flow**
```text
CPU Request (Virtual Addr)
       |
       v
[ MMU Check Page Table ]
       |
   Is Valid?
   /       \
 YES        NO ---> [ PAGE FAULT ]
  |          |       1. Penalty (Disk Read)
  |          |       2. Find Free Frame (or Evict LRU)
  |          |       3. Update Page Table
  |          |
  v          v
[ Get Physical Frame ]
       |
       v
[ Access Cache / RAM ]
```

---

## 5. Cache Hierarchy & Replacement Policy

To speed up access, the system uses a two-level cache between the CPU and RAM.

### 5.1 Hierarchy Structure
* **L1 Cache:** Small and extremely fast (1 cycle latency). Checked first.
* **L2 Cache:** Larger but slower (6 cycle latency). Checked if L1 misses.
* **Set Associativity:** Caches are divided into "Sets." A memory block can only go into a specific set based on its address. This mimics real hardware behavior.

### 5.2 Replacement Policy: FIFO (First-In, First-Out)
When a Cache Set is full and new data needs to be loaded, the system must evict an old block.
* **Policy:** The simulator uses **FIFO**.
* **Logic:** The block that entered the cache *earliest* is removed first. This is implemented using a queue structure for each set.

---

## 6. Page Replacement Policy (Virtual Memory)

Unlike the cache (which uses FIFO), the Virtual Memory system uses **LRU (Least Recently Used)** when RAM is full.

* **Why LRU?** In Virtual Memory, keeping "popular" pages in RAM is critical because disk access is incredibly slow compared to RAM access.
* **Implementation:** Every time a page is accessed, a global `last_used` timestamp is updated in the Page Table Entry.
* **Eviction:** When a frame is needed, the system inspects all pages currently in RAM and evicts the one with the **oldest** timestamp.

---

## 7. Limitations and Simplifications

While this simulator is comprehensive, it simplifies certain aspects of a real OS:

1.  **No Hardware TLB:** In a real CPU, a "Translation Lookaside Buffer" caches page table entries to speed up translation. We simulate the Page Table lookup directly.
2.  **Single-Level Paging:** We use a simple flat array for Page Tables. Real Operating Systems often use Multi-level paging (e.g., Page Directory -> Page Table) to save space.
3.  **Instruction Execution:** We simulate memory *accesses* (reads/writes) but do not actually execute assembly instructions or binary code.
4.  **Static PIDs:** Processes are manually initialized via the menu, whereas a real OS spawns them dynamically via system calls like `fork()` or `exec()`.

---

## 8. Project Structure
```text
MemorySimulator/
├── include/                 # Header files (buddy.h, cache.h, vm.h)
├── src/                     # Source code (Implementation logic)
│   ├── buddy.cpp            # Buddy System Logic
│   ├── cache.cpp            # L1/L2 Cache Logic
│   ├── main.cpp             # User Interface (CLI)
│   ├── memory.cpp           # Linear Allocator & RAM
│   └── vm.cpp               # Virtual Memory & Page Tables
├── test/                    # Input files for testing scenarios
├── output/                  # Log files generated by tests
├── run_all_tests.bat        # Windows automated test runner
├── run_all_tests.sh         # Linux/Mac automated test runner
├── Makefile                 # Compilation script
└── Readme.md                # Project documentation
```
