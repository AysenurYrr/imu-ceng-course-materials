#include "common.h"

/* Initialize shared memory buffer */
ShmBuf* model_init(void) {
    ShmBuf *shmp;

    // Reset all last read offsets
    for (int i = 0; i < MAX_SHELLS; i++) {
        last_read_offset[i] = 0;
    }

    int fd = shm_open(SHARED_FILE_PATH, O_CREAT | O_RDWR, 0600);
    if (fd < 0) {
        errExit("could not open shared file");
    }

    ftruncate(fd, BUF_SIZE);
    shmp = mmap(NULL, BUF_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (shmp != MAP_FAILED) {
        shmp->fd = fd;
        shmp->cnt = 0;
        shmp->shell = 0; // Default: broadcast
        sem_init(&shmp->sem, 1, 1);
    } else {
        errExit("mmap error");
    }

    return shmp;
}

/* Tokenize a shell command into argv-style arguments */
void parse_args(const char *command, char **args, int max_args) {
    if (!command || !args) return;
    char *cmd_copy = strdup(command);
    if (!cmd_copy) return;

    int i = 0;
    char *saveptr;
    char *token = strtok_r(cmd_copy, " \t\n", &saveptr);
    while (token && i < max_args - 1) {
        args[i++] = token;
        token = strtok_r(NULL, " \t\n", &saveptr);
    }
    args[i] = NULL;
}

/* Execute a shell command */
int model_execute_command(ShellInstance *shell, const char *command) {
    if (!shell || !command || strlen(command) == 0) {
        return 0;
    }

    // Handle cd command
    if (strncmp(command, "cd ", 3) == 0 || strcmp(command, "cd") == 0) {
        char path[256] = "";
        if (strlen(command) > 3) {
            strncpy(path, command + 3, sizeof(path) - 1);
        } else {
            strcpy(path, getenv("HOME"));
        }
        if (chdir(path) != 0) {
            char msg[512];
            snprintf(msg, sizeof(msg), "cd: '%s' dizinine geçilemedi\n", path);
            view_update_terminal(shell, msg);
        }
        return 0;
    }

    // Check for redirection
    char cmd_copy[256];
    strncpy(cmd_copy, command, sizeof(cmd_copy) - 1);
    cmd_copy[sizeof(cmd_copy) - 1] = '\0';

    char *redirect_pos = strstr(cmd_copy, ">>");
    int append_mode = 1;
    
    if (!redirect_pos) {
        redirect_pos = strchr(cmd_copy, '>');
        append_mode = 0;
    }

    char *filename = NULL;
    if (redirect_pos) {
        *redirect_pos = '\0';  // Split command at > or >>
        filename = redirect_pos + (append_mode ? 2 : 1);
        while (*filename == ' ') filename++;  // Skip spaces
    }

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return -1;
    }

    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        return -1;
    } else if (pid == 0) {
        close(pipefd[0]);

        if (filename) {
            // Open file for append or overwrite
            int flags = O_WRONLY | O_CREAT;
            flags |= append_mode ? O_APPEND : O_TRUNC;
            int fd = open(filename, flags, 0644);
            if (fd == -1) {
                perror("open");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        } else {
            dup2(pipefd[1], STDOUT_FILENO);
        }
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);

        char *args[64];
        parse_args(cmd_copy, args, 64);  // Use cmd_copy instead of command
        if (args[0]) {
            execvp(args[0], args);
            perror("execvp failed");
        }
        exit(EXIT_FAILURE);
    } else {
        close(pipefd[1]);

        if (shell->process_count < MAX_PROCESS_INFO) {
            shell->processes[shell->process_count].pid = pid;
            strncpy(shell->processes[shell->process_count].command, command, 255);
            shell->processes[shell->process_count].status = 1;
            shell->process_count++;
        }

        char buffer[BUF_SIZE] = {0};
        fcntl(pipefd[0], F_SETFL, O_NONBLOCK);

        fd_set readfds;
        struct timeval tv;
        int ready;

        FD_ZERO(&readfds);
        FD_SET(pipefd[0], &readfds);
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        ready = select(pipefd[0] + 1, &readfds, NULL, NULL, &tv);

        if (ready > 0) {
            char chunk[1024];
            ssize_t chunk_size;
            size_t total_read = 0;

            while ((chunk_size = read(pipefd[0], chunk, sizeof(chunk) - 1)) > 0) {
                chunk[chunk_size] = '\0';
                if (total_read + chunk_size >= BUF_SIZE - 1) {
                    buffer[total_read] = '\0';
                    view_update_terminal(shell, buffer);
                    total_read = 0;
                }
                strncpy(buffer + total_read, chunk, chunk_size);
                total_read += chunk_size;

                FD_ZERO(&readfds);
                FD_SET(pipefd[0], &readfds);
                tv.tv_sec = 0;
                tv.tv_usec = 100000;
                ready = select(pipefd[0] + 1, &readfds, NULL, NULL, &tv);
                if (ready <= 0) break;
            }

            if (total_read > 0) {
                buffer[total_read] = '\0';
                view_update_terminal(shell, buffer);
            }
        }

        close(pipefd[0]);
        int status;
        waitpid(pid, &status, 0);

        for (int i = 0; i < shell->process_count; i++) {
            if (shell->processes[i].pid == pid) {
                shell->processes[i].status = 0;
                break;
            }
        }

        return status;
    }
}

/* Send a message to the shared buffer */
int model_send_message(ShmBuf *shmp, const char *message) {
    if (!shmp || !message) return -1;

    sem_wait(&shmp->sem);

    // Create new message
    Message new_msg;
    new_msg.target_shell = shmp->shell;  // Store target information
    
    time_t now = time(NULL);
    strftime(new_msg.timestamp, sizeof(new_msg.timestamp), 
             "[%H:%M:%S] ", localtime(&now));
    
    strncpy(new_msg.content, message, sizeof(new_msg.content) - 1);

    // If buffer is full, shift messages
    if (shmp->cnt >= BUF_SIZE/sizeof(Message) - 1) {
        memmove(&shmp->messages[0], &shmp->messages[1], 
                (shmp->cnt - 1) * sizeof(Message));
        shmp->cnt--;
    }

    // Add new message
    shmp->messages[shmp->cnt++] = new_msg;
    
    sem_post(&shmp->sem);
    return 0;
}

/* Read messages from the shared buffer based on shell_id */
char* model_read_messages(ShmBuf *shmp, int shell_id) {
    static char buffer[BUF_SIZE];
    if (!shmp) return NULL;

    sem_wait(&shmp->sem);
    
    buffer[0] = '\0';
    size_t pos = 0;

    // Iterate through all messages
    for (size_t i = 0; i < shmp->cnt; i++) {
        Message *msg = &shmp->messages[i];
        
        // Show message if it's broadcast or specifically for this shell
        if (msg->target_shell == 0 || msg->target_shell == shell_id + 1) {
            int written = snprintf(buffer + pos, BUF_SIZE - pos, 
                                 "%s%s\n", msg->timestamp, msg->content);
            if (written > 0) {
                pos += written;
            }
        }
    }

    sem_post(&shmp->sem);
    return buffer;
}

/* Clean up model resources */
void model_cleanup(ShmBuf *shmp) {
    if (shmp) {
        sem_destroy(&shmp->sem);
        munmap(shmp, BUF_SIZE);
        shm_unlink(SHARED_FILE_PATH);
    }
}