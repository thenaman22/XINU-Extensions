# Demand Paging in Xinu

This project implements demand paging and virtual memory support in the Xinu operating system. It was developed as part of an operating systems course and extends the base Xinu kernel with paging mechanisms, memory-mapped file support, private process heaps, and a Second-Chance (SC) page replacement policy.

## Features

- **Virtual Memory Support**: Each process can access a 4GB virtual address space, with on-demand page table allocation.
- **Memory-Mapped Files**: Implemented `xmmap()` and `xmunmap()` system calls to allow user-space processes to map virtual pages to emulated backing stores.
- **Private Heaps**: `vcreate()` enables processes with isolated virtual heaps. Memory allocation and deallocation are handled via `vgetmem()` and `vfreemem()`.
- **Backing Store Management**: 
  - `get_bs()`, `release_bs()`, `read_bs()`, and `write_bs()` manage 8 emulated backing stores, each up to 1MB.
  - Backing stores simulate disk paging within physical memory due to Xinu's lack of file system support.
- **Page Replacement Policy**: Implemented Second-Chance (SC) algorithm to handle frame eviction when physical memory is full.
- **Inverted Page Table**: Tracks mapping of frames to (pid, vpage), supporting efficient page lookup and replacement.
- **Page Fault Handler**: Custom interrupt service routine (ISR) for interrupt 14 to handle demand paging and frame allocation.

## System Calls Implemented

- `SYSCALL xmmap(int virtpage, bsd_t source, int npages)`
- `SYSCALL xmunmap(int virtpage)`
- `SYSCALL vcreate(...)`
- `WORD* vgetmem(unsigned int nbytes)`
- `SYSCALL vfreemem(struct mblock* block, unsigned int size)`
- `SYSCALL srpolicy(int policy)`

## Memory Layout (Simplified)

|----------------------------|
| Virtual Memory (User)     |
|----------------------------|
| 8 Backing Stores (2MB-4MB)|
|----------------------------|
| Page Tables & Frames      |
|----------------------------|
| Kernel Memory             |
|----------------------------|
| Xinu Code & Global Memory |
|----------------------------|

## Testing
Test cases are included in testmain.c, verifying:

Shared memory access across processes using xmmap()

Allocation and deallocation from private heaps

Demand paging behavior and correctness of the replacement policy

Sample output is included under /sample_output.

## How to Run
Compile the code:

cd compile/
make clean
make


Use testmain.c to trigger paging events and verify system call behavior.

## Project Structure
/paging – Contains all virtual memory-related files (e.g., xm.c, vgetmem.c, policy.c)

/sys – Core Xinu syscall files, modified only where required

/include – Definitions and paging-related structs (e.g., pd_t, pt_t in paging.h)

testmain.c – Test harness for validating paging behavior

## Acknowledgements
This project was developed as part of CSC 501 at North Carolina State University. It builds upon the Xinu public release and extends it with modern memory management features.
