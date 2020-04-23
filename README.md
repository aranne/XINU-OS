# XINU-OS

[An educational OS](https://xinu.cs.purdue.edu): XINU stands for Xinu Is Not Unix -- although it shares concepts and even names with Unix, the internal design differs completely. 

### 1. Getting Acquainted with XINU

Xinu is a small, elegant operating system that supports dynamic process creation, dynamic memory allocation, network communication, local and remote file systems, a shell, and device-independent I/O functions. The small size makes Xinu suitable for embedded environments.

### 2. Process Scheduling

The default scheduler in Xinu will schedule the process with the higher priority. Starvation is produced in Xinu when there are two or more processes eligible for execution that have different priorities. The higher priority process gets to execute first which results in lower priority processes never getting any CPU time unless the higher priority process ends.

* A random scheduler 
* A Linux-Like scheduler

### 3. Reader/Writer Lock with Priority Inheritance

Readers/writer locks are used to synchronize access to a shared data structure. 
A lock can be acquired for read or write operations.
A lock acquired for reading can be shared by other readers, but a lock acquired for writing must be exclusive.
A priority inheritance mechanism is used to prevent the priority inversion problem when using locks.

* Lock operations
* Priority inheritance

### 4. Demand Paging

Demand paging is a method of mapping a large address space into a relatively small amount of physical memory. It allows a program to use an address space that is larger than the physical memory, and access non-contiguous sections of the physical memory in a contiguous way. Demand paging is accomplished by using a “backing store” (usually disk) to hold pages of memory that are not currently in use.

* Memory mapping
* Demand paging
* Backing store management
* Page replacement policies: SC, LFU
