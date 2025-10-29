[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/B6NzQ-1j)
# Lab Assignment: Asynchronous I/O with Signals, Pipes, and Network Sockets
- [Lab Assignment: Asynchronous I/O with Signals, Pipes, and Network Sockets](#lab-assignment-asynchronous-io-with-signals-pipes-and-network-sockets)
  - [**Prerequisites**](#prerequisites)
  - [Learning Objectives](#learning-objectives)
  - [Details](#details)
  - [**Task 1: Asynchronous I/O with Pipes**](#task-1-asynchronous-io-with-pipes)
  - [**Task 2: Understanding Signals**](#task-2-understanding-signals)
  - [**Task 3: Network Server with `epoll()`**](#task-3-network-server-with-epoll)
    - [Requirements:](#requirements)
  - [**Deliverables**](#deliverables)
  - [**Bonus Challenge**](#bonus-challenge)
  - [**Submission**](#submission)
  - [Provided files](#provided-files)
  - [Important notes on signal safety](#important-notes-on-signal-safety)
    - [**1. Why `read()`/`write()` Are Unsafe in Signal Handlers**](#1-why-readwrite-are-unsafe-in-signal-handlers)
    - [**2. Safe Alternatives for Signal Handlers**](#2-safe-alternatives-for-signal-handlers)
    - [**3. When `read()`/`write()` Might Work (But Still Risky)**](#3-when-readwrite-might-work-but-still-risky)
    - [**Key Takeaways**](#key-takeaways)
    - [**Further Reading**](#further-reading)

## **Prerequisites**  
- Understanding of **signals** (e.g., `SIGIO`).  
- Familiarity with **pipes** and **file descriptors**.  
- Basic knowledge of **socket programming**.  #

## Learning Objectives
- Asynchronous I/O by using signal programming, 
- Extend the concept to handle multiple clients using **epoll()** with network sockets.  

## Details
In this lab, you are going to design a SIGIO handler for asynchronous I/O and combine this with pipes to do a kind of asynchronous communication.

Using fcntl(2) - Linux manual page , you can change settings on file descriptors:
 ``fcntl(fd, F_SETOWN, getpid());``
  - This changes who gets SIGIO when a change happens on fd. Another setting is
``fcntl(fd, F_SETFL, O_ASYNC);``
  - This enables generations of SIGIO on fd. 
``fcntl(fd, F_SETFL, O_NONBLOCK);``
  - This makes calls like ``read(fd,..)`` nonblocking.

Therefore, we can design a program that is informed when an input is given to fd.

## **Task 1: Asynchronous I/O with Pipes**  
Modify the provided `aio_example.c` to use **pipes** instead of stdin (`fd=0`). The parent process must asynchronously read messages written by a child process via a pipe.  
1. **Parent Process**:  
   - Set up a pipe and configure its read-end (`pipe_fd[0]`) to generate `SIGIO` when data is available.  
   - Use `fcntl` with `F_SETOWN` and `F_SETFL` (flags: `O_ASYNC`, `O_NONBLOCK`).  
   - Implement a `SIGIO` handler to trigger non-blocking reads.  

2. **Child Process**:  
   - Write a message (e.g., `"Score: 20"`) to the pipe after a short delay (use `sleep(1)`).  
   - Close the pipe after writing.  

3. **Output**:  
   The parent must print `"Parent received: [message]"` when it reads data from the pipe.  

**Important note:** do not use `printf` or `read/write` etc. in signals. Instead set flags.  
```c
volatile sig_atomic_t flag = 0;  // Must be sig_atomic_t!

void handler(int sig) {
    flag = 1;  // Safe: Assignment is atomic
}
```

**Hints:** 
- Use `fork()` to create the child process.  
- Refer to `pipes_example.c` for pipe setup.  
- Ensure proper cleanup of pipe file descriptors. 

## **Task 2: Understanding Signals**  
1. Why is `volatile sig_atomic_t` used for the `canread` flag?  
2. What happens if `O_NONBLOCK` is not set on the pipe’s read-end?  
3. Explain the role of `fcntl(fd, F_SETOWN, getpid())`.  

---

## **Task 3: Network Server with `epoll()`**  
Extend the lab to handle **multiple clients** using `epoll()`. Write a server (`server.c`) that:  
1. Listens on port `8080` for incoming TCP connections.  
2. Uses `epoll()` to monitor:  
   - The server socket for new connections.  
   - All client sockets for incoming data.  
3. **Echo Back**: When a client sends data, the server echoes it back to the client.  

### Requirements:  
1. **Server Setup**:  
   - Create a TCP socket, bind it to `127.0.0.1:8080`, and listen.  
   - Add the server socket to the `epoll` interest list.  

2. **Client Handling**:  
   - Use `epoll_wait()` to detect events.  
   - For new connections, add the client socket to the `epoll` list.  
   - For incoming data, read and echo it back.  

3. **Output**:  
   Print `"New client connected: [fd]"` and `"Client [fd] sent: [message]"` for debugging.  

**Hints**:  
- Use `epoll_create1()`, `epoll_ctl()`, and `epoll_wait()`.  
- Handle edge cases (e.g., client disconnects).  

---

## **Deliverables**  
1. Modified `aio_example.c` for Task 1.  
2. `server.c` for Task 3.    

---

## **Bonus Challenge**  
- Use `select()` instead of `epoll()`  and compare performance.  
- Handle partial reads/writes in the server.  

---

## **Submission**  
commit all of your files in your repo 

**Lab Notes**:  
- Test your code thoroughly for file descriptor leaks.  
- Use `strace` or `perf` to debug signal handling.  
- Refer to `man epoll` and `man fcntl` for documentation.

## Provided files

**aio_example.c**
```c
/* aio_example.c*/
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* See notes on why "volatile sig_atomic_t" is better*/
volatile sig_atomic_t canread = 0;

void sigio_handler(int signo) {
    canread = 1;
    return;
}
void spin_for_io() {
    while (!canread) {
        // sleep(1);
    }
}


int main() {
    /*set owner for fd*/
    fcntl(0, F_SETOWN, getpid());
    /*enable SIGIO on fd*/
    fcntl(0, F_SETFL, O_ASYNC);

    /*set signal handler*/
    struct sigaction action = {0};
    action.sa_handler = &sigio_handler;

    /*you can also use sa_sigction
    action.sa_sigaction = &sigio_handler;//need to change function prototype
    */
    if (sigaction(SIGIO, &action, NULL) == -1) {
        perror("signal action");
        exit(EXIT_FAILURE);
    }

    canread = 0;
    char buf[256];

    for (;;) {
        spin_for_io(); /*you may be doing smthing else*/
        while (read(0, buf, 256) > 0) {
            printf("your msg: %.256s\n", buf);
            fflush(stdout);
        }
    }
}
```
**pipes_example.c**
```c
/*
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

int main() {
    int fd[2];
    /* Called with the O_CLOEXEC flag to prevent any commands from blocking */
    pipe(fd);
    fcntl(fd[0], F_SETFL, O_NONBLOCK);

    printf("started\n");
    pid_t p = fork();
    if (p > 0) {
        int score;
        printf("started\n");

        char buf[1024] = {0};
        read(fd[0], buf, 1024);
        printf("read did not wait, because of O_NONBLOCK\n");
        printf("The child says: %.1024s\n", buf);
    } else {
        sleep(1);/*sleep to finish after parent*/
        char msg[1024] = {0};
        sprintf(msg, "Score is %d", 10 + 10);
        write(fd[1], msg, 1024);
        printf("child done\n");
    }
    
    close(fd[0]);
    close(fd[1]);
    return 0;
}
```

**test_client.c**: You can use this to test your server.
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define MSG_INTERVAL 2  // Seconds between messages
#define MAX_MSGS 5      // Messages per client

void client_process(int client_id) {
    int sockfd;
    struct sockaddr_in server_addr;

    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Connect to server
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Client %d connected to server\n", client_id);

    // Send/receive loop
    char buf[1024];
    for (int i = 0; i < MAX_MSGS; i++) {
        // Send message
        int len = snprintf(buf, sizeof(buf), "Client %d: Message %d", client_id, i+1);
        write(sockfd, buf, len);
        printf("Client %d sent: %s\n", client_id, buf);

        // Receive echo
        ssize_t bytes = read(sockfd, buf, sizeof(buf));
        if (bytes <= 0) {
            printf("Client %d: Server disconnected\n", client_id);
            break;
        }
        buf[bytes] = '\0';
        printf("Client %d received: %s\n", client_id, buf);

        sleep(MSG_INTERVAL);
    }

    close(sockfd);
    printf("Client %d exiting\n", client_id);
}

int main(int argc, char *argv[]) {
    int num_clients = 1;

    if (argc > 1) {
        num_clients = atoi(argv[1]);
        if (num_clients <= 0) {
            fprintf(stderr, "Invalid client count. Using 1 client.\n");
            num_clients = 1;
        }
    }

    // Spawn client processes
    for (int i = 0; i < num_clients; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            client_process(i + 1);  // Child process
            exit(EXIT_SUCCESS);
        } else if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
    }

    // Wait for all children to exit
    while (wait(NULL) > 0);
    printf("All clients finished\n");
    return 0;
}
```

---------------------

## Important notes on signal safety

### **1. Why `read()`/`write()` Are Unsafe in Signal Handlers**  
Signal handlers must be **async-signal-safe**—meaning they can safely execute even if interrupted mid-operation. The POSIX standard lists [async-signal-safe functions](https://man7.org/linux/man-pages/man7/signal-safety.7.html), and **`read()`/`write()` are technically async-signal-safe**, but:  

**Issues with `read()`/`write()` in Handlers**  
| Problem | Explanation |  
|---------|------------|  
| **Reentrancy** | If the main thread is already in `read()`/`write()` when a signal arrives, the handler’s call could corrupt internal state. |  
| **Blocking** | If the file descriptor is in a state that would block (e.g., no data available), the handler may hang indefinitely. |  
| **Global State Corruption** | Buffered I/O (e.g., `stdio` internals) is not atomic. A signal interrupting `printf()` could leave locks held. |  

---

### **2. Safe Alternatives for Signal Handlers**  

**Option 1: Set a Flag and Defer I/O**  
```c
volatile sig_atomic_t flag = 0;  // Must be sig_atomic_t!

void handler(int sig) {
    flag = 1;  // Safe: Assignment is atomic
}

int main() {
    while (1) {
        if (flag) {
            flag = 0;
            char buf[256];
            ssize_t n = read(STDIN_FILENO, buf, sizeof(buf));  // Safe outside handler
            if (n > 0) write(STDOUT_FILENO, buf, n);           // Safe outside handler
        }
    }
}
```

**Option 2: Use `signalfd()` (Linux-Specific)**  
```c
#include <sys/signalfd.h>
// Block signals and read them via a file descriptor (no handler needed!)
sigset_t mask;
sigemptyset(&mask);
sigaddset(&mask, SIGIO);
sigprocmask(SIG_BLOCK, &mask, NULL);

int sfd = signalfd(-1, &mask, 0);
// Now poll/read sfd like a normal file descriptor
```

**Option 3: Self-Pipe Trick**  
```c
int pipefd[2];
pipe(pipefd);

void handler(int sig) {
    write(pipefd[1], "X", 1);  // Safe: write() to pipe is async-signal-safe
}

// In main():
char buf[1];
read(pipefd[0], buf, 1);  // Deferred I/O
```

---

### **3. When `read()`/`write()` Might Work (But Still Risky)**  
- **Pipes/Sockets**: `write()` to a non-blocking pipe/socket is usually safe.  
- **Raw File Descriptors**: Avoid buffered I/O (e.g., `FILE*`, `printf`).  

**Example (Still Not Recommended)**  
```c
void handler(int sig) {
    const char msg[] = "Got signal!\n";
    write(STDERR_FILENO, msg, sizeof(msg) - 1);  // Technically safe, but risky
}
```

---

### *Key Takeaways*
| Approach | Safety | Recommendation |  
|----------|--------|----------------|  
| **Flag + Deferred I/O** | ✅ Safe | Best for portability |  
| **`signalfd()`** | ✅ Safe (Linux) | Best for event loops |  
| **Self-Pipe** | ✅ Safe | Classic Unix pattern |  
| **Direct `read()`/`write()`** | ❌ Risky | Avoid unless absolutely necessary |  

**Rule of Thumb**: **Never** call non-trivial functions in signal handlers. Use the handler only to set flags or notify via async-signal-safe mechanisms.  

---

### **Further Reading**  
- POSIX Async-Signal-Safe Functions: [`man 7 signal-safety`](https://man7.org/linux/man-pages/man7/signal-safety.7.html)  
- `signalfd()`: [`man 2 signalfd`](https://man7.org/linux/man-pages/man2/signalfd.2.html)  
- Self-Pipe Trick: [Wikipedia](https://en.wikipedia.org/wiki/Self-pipe_trick)
