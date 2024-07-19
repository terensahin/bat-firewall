## Processes and Programs
A program is a file containing a range of information that describes how to construct a process at run time.
A process is an instance of an executing program. From the kernel’s point of view, a process consists of user-space
memory containing program code and variables used by that code, and a
range of kernel data structures that maintain information about the state
of the process


## Process ID and Parent Process ID
- Each process has a process ID (PID), a positive integer that uniquely identifies the process on the system.
- The getpid() system call returns the process ID of the calling process.
- Each process has a parent—the process that created it. A process can find out the process ID of its parent using the getppid() system call.
- It is conventional to say that init has no parent and that, therefore, the PPID value of 0 is a placeholder signaling that it has no parent.

## Memory layout of a process
The memory allocated to each process is composed of a number of parts, usually referred to as segments. These segments are as follows:
- The text segment contains the machine-language instructions of the program
run by the process. The text segment is made read-only so that a process
doesn’t accidentally modify its own instructions via a bad pointer value.
- The initialized data segment contains global and static variables that are explic-
itly initialized.
- The uninitialized data segment contains global and static variables that are not
explicitly initialized. Before starting the program, the system initializes all
memory in this segment to 0.
- The stack is a dynamically growing and shrinking segment containing stack
frames. One stack frame is allocated for each currently called function.
- The heap is an area from which memory (for variables) can be dynamically allo-
cated at run time.




## Memory between processes
Where appropriate, two or more processes can share memory. The kernel
makes this possible by having page-table entries in different processes refer to
the same pages of RAM. Memory sharing occurs in two common circumstances:
–
Multiple processes executing the same program can share a single (read-
only) copy of the program code. This type of sharing is performed implicitly
when multiple programs execute the same program file (or load the same
shared library).
–
Processes can use the shmget() and mmap() system calls to explicitly request
sharing of memory regions with other processes. This is done for the pur-
pose of interprocess communication.

## Stack and stack frame
The stack consists of a series of frames, with a new frame being added as a
function is invoked and removed when the function returns. Each frame contains
the local variables, function arguments, and call linkage information for a single
function invocation. A special-purpose register, the stack pointer, tracks the current top of
the stack.


## Environment
Environment variables are a set of dynamic values that can affect the way running processes behave on a computer. They are part of the environment in which a process runs and can be used to pass configuration information to applications and services.
- Access: Use getenv to get the value of an environment variable.
- Set: Use setenv (POSIX) or _putenv (Windows) to set an environment variable.
- List: Use the environ global variable to list all environment variables (POSIX).


## fork()

A process can create a new process using the fork() system call. The process that calls fork() is referred to as the parent process, and the new process is referred to as the child process. The kernel creates the child process by making a duplicate of the parent process. The child inherits copies of the parent’s data, stack, and heap segments, which it may then modify independently of the parent’s copies.

The text is always shared between parent and child (if not exec() system calls are used) but other shared parts are seperated when any of these processes tries to write in it. This algorithm is called COW(copy on write)



## What a child process inherits
| **Attribute**                   | **Inherits**                   |
|---------------------------------|--------------------------------|
| Process ID (PID)                | ✗                              |
| Parent Process ID (PPID)        | ✓                              |
| File Descriptors                | ✓                              |
| Environment Variables           | ✓                              |
| Memory                          | ✓ (copy)                       |
| Program Counter                 | ✓ (execution continues)        |
| Signal Handlers                 | ✓                              |
| User and Group IDs              | ✓                              |
| Current Working Directory       | ✓                              |
| File Mode Creation Mask (umask) | ✓                              |
| Resource Limits                 | ✓                              |
| Control Terminal                | ✓                              |
| Process Group and Session       | ✓                              |
| Pending Signals                 | ✗                              |
| Locks on Files                  | ✗                              |


## Daemons
A daemon is a process with the following characteristics:
- It is long-lived. Often, a daemon is created at system startup and runs until the
system is shut down.
- It runs in the background and has no controlling terminal. The lack of a control-
ling terminal ensures that the kernel never automatically generates any job-control
or terminal-related signals (such as SIGINT, SIGTSTP, and SIGHUP) for a daemon.

In UNIX, the names of daemons conventionally end in "d".

**To become a daemon, a program performs the following steps:**
- Perform a fork(), after which the parent exits and the child continues.
- The child process calls setsid() to start a new session and free itself of any
association with a controlling terminal
- If the daemon might later open a terminal device, then we must take steps to ensure that the device does not become the controlling terminal
- Clear the process umask to ensure that, when the daemon creates files and
directories, they have the requested permissions.
- Change the process’s current working directory, typically to the root
directory
- Close all open file descriptors that the daemon has inherited from its parent
- After having closed file descriptors 0, 1, and 2, a daemon normally opens
/dev/null and uses dup2() (or similar) to make all those descriptors refer to
this device
