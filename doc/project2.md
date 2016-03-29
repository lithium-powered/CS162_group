Design Document for Project 2: User Programs
============================================

## Group Members

* Albert Weng <albertweng@berkeley.edu>
* Annie Lin <annie_lin@berkeley.edu>
* Liquin Yu <liyu@berkeley.edu>
* Nerissa Lin <lin.nerissa@berkeley.edu>

# Proposed Design

## Task 1: Argument Passing

### 1) Data Structures and Functions:
Add into `process.c`
```
Void pushMainArgs(const char *file_name, void **esp){
}
```

### 2) Algorithm:
- Step 1: We catch the process start up in the load function.
- Step 2: We take the file_name string which contains the commandline arguments into pushMainArgs.
- Step 3: pushMainArgs will take in the file_name and splits it on spaces. We then push these arguments onto the stack along with their addresses. Adding on the fake return value as needed etc.

### 3) Synchronization: 
Need to make sure pushing things onto the stack is atomic. We cannot have another thread run while this current process is being loaded.

### 4) Rationale: 

We only need to take sure that we push arguments onto the stack, we shouldn’t check if they are valid arguments here because we can’t be sure without the process running.

## Task 2: Process Control Syscalls

### 1) Data Structures and Functions
```
void copyArguments(%esp){}
void execute(syscall, [args]){}
```

### 2) Algorithm:
- Step 1: Thread makes a syscall with appropriate arguments
- Step 2: OS checks if user stack pointer is invalid or too close to page boundary, then copies arguments supplied by the caller onto the kernel stack and validates each argument
- Step 3: OS executes the syscall using syscall equivalent functions (from /lib/user/syscall.c):and returns the answer to the thread (or exits the thread if it was a terminal syscall)

### 3) Synchronization:
Only one thread should be writing to the stack at a time (but many threads can read at a time as long as readers and writers are not accessing the stack at the same time), so use conditional variables to make a readers and writers-style write-lock to the stack.

### 4) Rationale:
The implementation is very straightforward because we follow the syscall process described in lecture. The readers and writers conditional variables access to the stack ensures thread-safe stack writes without starving any threads (allowing many readers to access at the same time). We have been careful to account for all possible edge cases before copying arguments to the kernel to ensure we do not corrupt the kernel with bad data or invalid memory accesses from the process.

### 5) Edge cases:
Invalid memory accesses - null pointers, invalid pointers, pointers to kernel address space, pointers to other user program’s memory space for which the current program does not have permission to access, half-valid accesses on page boundaries
We should terminate user program in all of these cases.



## Task 3: File Operation Syscalls

### 1) Data Structures and Functions:
```
bool create (const char *file, unsigned initial size) {}
bool remove (const char *file) {}
int open (const char *file) {}
int filesize (int fd) {}
int read (int fd, const void *buffer, unsigned size) {}
int write (int fd, const void *buffer, unsigned size) {}
void seek (int fd, unsigned position) {}
unsigned tell (int fd) {}
void close (int fd) {}
```

### 2) Algorithms:
We implement each of these functions as cases in the syscall_handler and make sure for all the file system syscalls, we use global file system lock. To implement many of these functions we call the appropriate functions in syscall.c. We follow the specs to implement each of these functions and call them from syscall_handler.

### 3) Synchronization:
The main synchronization risk is that files are not threadsafe (only one thread can access a file at a time). We use a reader-writer-style conditional variable system to ensure only one thread is writing to a file at a time. Also, we organize our syscalls by a global lock so we make sure that it is thread-safe.

### 4) Rationale:
Our design is simple and closely follows the specs as we look to implement each function and call them within the syscall_handler. By using the recommended strategy of a global lock, we guarantee thread safety. However, this is a very simple synchronization and as a result we may be losing some performance.

### 5) Edge Cases: 
Shared files between multiple user programs?


# Questions:

### Question 1: 
Name of test: sc-bad-sp in file sc-bad-sp.c
This test invokes movl to copy an invalid address (an address significantly below the code segment) into the stack pointer %esp (line 18). It then tries to execute a syscall by initializing a new int variable and storing it on the stack (line 18). Because the process should not be allowed to write to user process memory at an invalid stack pointer address, the test expects the process to exit immediately with a -1 exit code (signifying an error).
### Question 2:
Name of test: sc-bad-arg in file sc-bad-arg.c
This test moves the stack pointer %esp to the very top of the stack and then places the syscall number for SYS_EXIT at the top of the stack where the stack pointer is pointing (line 14). It then attempts to invoke the syscall (line 15). Because the arguments for the syscall are stored after the syscall itself in memory, the arguments to this SYS_EXIT call are above the user process space and should not be allowed to be accessed; thus, the test expects the process to exit immediately with a -1 exit code (signifying an error)
### Question 3:
I don’t know!


