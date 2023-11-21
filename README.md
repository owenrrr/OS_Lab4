## Experiment Goal

**Refer to Chapter 6 of "Orange's" to implement the simulation of specific process scheduling problems on the previously built nasm+bochs experimental platform.**



### 1. Description

Add a system call, which accepts int type parameter milli_seconds. The process calling this method will wait for milli_seconds
No time slice is allocated within milliseconds.
– Note that the code in Chapter 6 already has the method mills_delay in clock.c. This method is still for the process.
A time slice is allocated, but the process enters an empty loop.
• Add system call to print string, accepting char* type parameter str
– Note that the code in Chapter 6 already has the disp_str function in the kliba.asm file to display strings, but this
is a kernel function. Please implement and wrap it into the corresponding system call.
• Add two system calls to perform semaphore PV operations, and simulate the reader-writer problem on this basis.
– There are 6 processes that have always existed (cyclic read and write operations). A, B, and C are reader processes, and D and E are writer processes.
process, F is an ordinary process, where
∗ A reading consumes 2 time slices
∗ B and C reading consumes 3 time slices
∗ D write consumes 3 time slices
∗ E writing consumes 4 time slices
– While readers are reading, writers cannot write until all readers have finished reading.
–Only one author can be writing at the same time
– While writing, the reader cannot read
– Multiple readers can read a book, but not too many. The upper limit is 1, 2, or 3. All readers can support it if needed.
and can be modified on-site
– Processes A, B, C, D, and E require basic operations for color printing: read start, write start, read, write, read completion,
The writing is completed, and the corresponding process name
-F prints every time slice whether it is reading or writing, and if it is reading, how many people are there
– Please implement reader priority and writer priority respectively. Both needs can be supported and can be modified on-site.
– Please find a way to solve the process starvation problem in some cases of this problem (please refer to Chapter 6)



### 2. Tips

• Build the entire project using make or similar tools. The makefile must support the make run command to start directly.
No other commands are required.
• This assignment can be completed directly based on the source code of "orange’s". Please record the additions or modifications.
• Please submit code, Makefile, documentation and screenshots



### 3. References

"Orange’s Implementation of an Operating System"



### 4. Assignment

- screenshot_1, screenshot_2

- EXPERIMENT_DOCUMENT.md: explain details about the experiment implementation 
