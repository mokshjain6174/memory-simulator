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
* **Coalescing (Merging):** When a block is freed, the system checks its "Buddy" (its neighbor in memory). If the Buddy
