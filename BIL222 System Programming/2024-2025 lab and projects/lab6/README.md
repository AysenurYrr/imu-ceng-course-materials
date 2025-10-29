[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/qLBEcb-G)

### Lab Session: Process Management and Implmenting Pipes Using Standard Files
- [Lab Session: Process Management and Implmenting Pipes Using Standard Files](#lab-session-process-management-and-implmenting-pipes-using-standard-files)
  - [**Learning Objectives**](#learning-objectives)
  - [**Prerequisites**](#prerequisites)
- [**Lab Tasks**](#lab-tasks)
- [**Task 0: Simulating Pipe Semantics Using a File (`temp.txt`)**](#task-0-simulating-pipe-semantics-using-a-file-temptxt)
  - [**Background**](#background)
  - [**Steps**](#steps)
  - [**Key Notes**](#key-notes)
  - [**Expected Output**](#expected-output)
  - [**Discussion Questions**](#discussion-questions)
  - [**Debugging Tips**](#debugging-tips)
- [**Task 1: Create a Pipe and Redirect Output**](#task-1-create-a-pipe-and-redirect-output)
- [**Task 2: Implement a Multi-Stage Pipeline (Challenge)**](#task-2-implement-a-multi-stage-pipeline-challenge)
  - [**Common Pitfalls \& Debugging Tips**](#common-pitfalls--debugging-tips)
- [**Assessment**](#assessment)
- [**Further Exploration**](#further-exploration)
- [**Lab Report Submission:**](#lab-report-submission)

---

#### **Learning Objectives**
By the end of this lab, students will be able to:
1. Create and manage processes using `fork()` and `exec()`.
2. Understand how standard I/O redirection works using files.  
3. Simulate pipe-like behavior by writing output to a file and
reading from it.  
1. Recognize the limitations of file-based "pipes" compared to
actual pipes.  
1. Implement inter-process communication (IPC) using unnamed pipes.
2. Redirect standard input/output (`stdin`/`stdout`) via `dup2`.
3. Build a simple shell-like pipeline (e.g., `ls | grep .c`).


---


#### **Prerequisites**

1. Basic understanding of C programming.
2. Familiarity with UNIX system calls (`fork`, `exec`,  `pipe`).
3. Knowledge of file descriptors and standard I/O.

---


### **Lab Tasks**

---

### **Task 0: Simulating Pipe Semantics Using a File (`temp.txt`)**  
**Duration:** 30 minutes  
**Objective:** Redirect the output of one process to a file
(`temp.txt`) and have another process read from it, mimicking a
basic pipe.  


---


#### **Background**  

In UNIX systems, **pipes** allow processes to communicate *in
memory* without intermediate files. Here, you will simulate this
behavior by:  
1. Writing output from Process A to `temp.txt`.  
2. Reading input for Process B from `temp.txt`.  

This approach is **not** equivalent to a true pipe (due to disk I/O
and lack of concurrency), but it demonstrates I/O redirection
mechanics.  


---

#### **Steps**  
1. **Create a temporary file**: Use `open()` to create `temp.txt`.  
2. **Fork the first child** (Writer Process):  
    - Redirect `stdout` to `temp.txt` using `dup2()`.  
    - Execute a command (e.g., `ls -l`).  
3. **Wait for the writer to finish**: Ensure data is fully
written.  
4. **Fork the second child** (Reader Process):  
    - Redirect `stdin` to `temp.txt` using `dup2()`.  
    - Execute a command to process the data (e.g., `wc -l`).  
5. **Cleanup**: Delete `temp.txt`.  



---


#### **Key Notes**  

1. **Synchronization**: The parent waits for the writer to finish
before starting the reader.  
2. **File Descriptors**:  
    - `dup2(fd, STDOUT_FILENO)` replaces `stdout` with the file descriptor for `temp.txt`.  
    - `dup2(fd, STDIN_FILENO)` replaces `stdin` with the file descriptor.  
3. **Cleanup**: Always close file descriptors and delete the
temporary file.  


---


#### **Expected Output**  

Running the program should output the number of lines from `ls -l`,
similar to:  

```

8

```


---


#### **Discussion Questions**  

1. **Limitations**:  
    - Why isn’t this approach equivalent to a real pipe?  
    - What happens if the reader starts before the writer finishes?  
2. **Real-World Use Cases**: When might redirecting I/O via files be
useful despite the overhead?  


---


#### **Debugging Tips**  

- Use `printf` (or `dprintf(fd, ...)`) to debug redirection.  
- Check file permissions and paths.  
- Verify the file content with `cat temp.txt` after the writer
finishes.  


---


**Next Step:** In the next part you’ll replace the file with an actual
pipe to enable concurrent communication!

---

### **Task 1: Create a Pipe and Redirect Output**
**Duration:** 30 minutes
**Objective:** Create a pipe and redirect the output of a parent
process to a child process using `dup2`.


**Steps:**

1. **Create a pipe** using `pipe(int fd[2])`.
2. **Fork a child process**.
3. **Parent Process**:
    - Close the read end of the pipe (`fd[0]`).
    - Redirect `stdout` to the write end of the pipe using `dup2(fd[1], STDOUT_FILENO)`.
    - Execute a command (e.g., `ls -l`) using `execlp`.
4. **Child Process**:
    - Close the write end of the pipe (`fd[1]`).
    - Redirect `stdin` to the read end of the pipe using `dup2(fd[0], STDIN_FILENO)`.
    - Execute a command to process input (e.g., `wc -l`).

**Sample Code:**

```c

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>


int main() {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { // Child: reads from pipe
    
    } else { // Parent: writes to pipe
    
    }

    return 0;
}

```


**Explanation:**

- The parent process writes `ls -l` output to the pipe.
- The child process reads from the pipe and counts lines with `wc
-l`.
- Use `dup2` to ensure `stdout`/`stdin` point to the pipe ends.

---


### **Task 2: Implement a Multi-Stage Pipeline (Challenge)**
**Duration:** 30 minutes
**Objective:** Chain three commands (e.g., `ls | grep .c | wc -l`)
using two pipes.


**Steps:**

1. Create two pipes.
2. Fork two child processes.
3. **First Child**:
    - Redirect `stdout` to the first pipe.
    - Execute `ls`.
4. **Second Child**:
    - Redirect `stdin` to the first pipe and `stdout` to the second pipe.
    - Execute `grep .c`.
5. **Parent**:
    - Redirect `stdin` to the second pipe.
    - Execute `wc -l`.



---


#### **Common Pitfalls & Debugging Tips**

1. **Close Unused File Descriptors**: Forgetting to close unused
pipe ends can cause hangs.
2. **Order of Operations**: Always redirect with `dup2` before
calling `exec`.
3. **Error Handling**: Check return values of `pipe`, `fork`, and
`dup2`.


---


### **Assessment**
0. **Task 0**: Successful execution of `ls -l | wc -l`.
1. **Task 1**: Successful execution of `ls -l | wc -l`.
2. **Task 2**: Correct chaining of three commands.
3. **Error Handling**: Graceful termination on failures.


---


### **Further Exploration**

1. Modify the code to handle bidirectional communication.
2. Implement a shell that supports arbitrary pipelines (e.g., `cmd1
| cmd2 | cmd3`).

---


### **Lab Report Submission:**  
Submit your code, screenshots of outputs, and a brief explanation of
how `dup2` and pipes work together.
