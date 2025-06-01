# CS537 – Introduction to Operating Systems (Fall 2022)

This repository contains coursework and programming assignments from CS537 at the University of Wisconsin–Madison. The course covers foundational operating system concepts with a focus on systems programming in C, multithreading, process management, and file systems.

---

## 🧠 Course Focus

- Concurrency and synchronization
- Thread scheduling and locking primitives
- System calls and memory management
- File system design and virtual memory
- Kernel-level vs user-level OS abstractions

---

## 📁 Key Projects & Implementations

### 🔹 Project 1: `p1a` / `p1b` – Process Scheduling
- Implemented custom user-level scheduling policies.
- Simulated scheduling algorithms like Round Robin and FCFS.
- Practiced low-level I/O redirection and process control.

### 🔹 Project 2: `p2a` / `p2b` – Multithreaded Sorting and Parallel MapReduce
- Built multithreaded versions of sorting and MapReduce algorithms.
- Applied pthreads, mutexes, and condition variables to manage concurrency.
- Gained practical experience with shared memory and synchronization bugs.

### 🔹 Project 3: `p3a` / `p3b` – Memory Allocation & Heap Management
- Reimplemented `malloc` and `free` to manage heap memory manually.
- Handled fragmentation, block splitting, and coalescing.
- Compared performance with the native allocator under memory stress tests.

### 🔹 Project 4: `p4a` / `p4b` – File System Simulation
- Created a simple file system abstraction with directories and files.
- Implemented path parsing, block management, and persistence.
- Simulated file operations like `mkdir`, `ls`, `cd`, and `write`.

---

## 💡 Learning Outcomes

- Built and debugged systems-level C programs from scratch.
- Understood OS internals like memory allocation, scheduling, and file handling.
- Gained hands-on experience in multithreading, synchronization, and system architecture.

---

## 🛠 Tools & Environment

- C, `gcc`, `make`
- `gdb`, `valgrind`, `perf`
- Linux/Unix development environment

---

## 📂 Repository Notes

Each subdirectory (`p1a`, `p2b`, etc.) contains the source code, test scripts, and reports for the corresponding project. This work reflects individual and team-based assignments focused on real-world OS design principles.

