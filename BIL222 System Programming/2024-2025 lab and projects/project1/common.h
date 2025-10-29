#ifndef COMMON_H
#define COMMON_H

#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <stdbool.h>

#define BUF_SIZE 8768
#define SHARED_FILE_PATH "mymsgbuf"
#define MAX_COMMAND_LENGTH 1024
#define MAX_SHELLS 10
#define MAX_PROCESS_INFO 100
#define MAX_HISTORY 100

#define errExit(msg)        \
    do {                    \
        perror(msg);        \
        exit(EXIT_FAILURE); \
    } while (0)

// Mesaj yapısını ekleyelim
typedef struct {
    int target_shell;  // 0=broadcast, 1-MAX_SHELLS=specific shell
    char content[512];
    char timestamp[32];
} Message;

typedef struct shmbuf {
    sem_t sem;         // Paylaşımlı belleğe eş zamanlı erişimi kontrol eden semaphore
    size_t cnt;        // Number of messages
    int fd;            // mmap yapılan dosyanın file descriptor'ı
    int shell;         // Mesajın gönderileceği shell ID'si (0 ise tüm shell'lere)
    Message messages[BUF_SIZE/sizeof(Message)];  // Array of messages instead of char buffer
} ShmBuf;


/* Process information structure */
typedef struct {
    pid_t pid;          /* Process ID of command */
    char command[256];  /* Command string */
    int status;         /* Running/exited */
} ProcessInfo;

/* Shell instance structure */
typedef struct {
    int id;
    GtkWidget *window;
    GtkWidget *terminal_view;
    GtkWidget *message_view;
    GtkWidget *entry;
    GtkTextBuffer *terminal_buffer;
    GtkTextBuffer *message_buffer;
    ProcessInfo processes[MAX_PROCESS_INFO];
    int process_count;

    /* history */ 
    char *history[MAX_HISTORY];
    int history_count;
    int history_index;
} ShellInstance;

/* Function declarations for model */
ShmBuf* model_init(void);
int model_execute_command(ShellInstance *shell, const char *command);
int model_send_message(ShmBuf *shmp, const char *message);
void model_cleanup(ShmBuf *shmp);
int model_send_dm(ShmBuf *shmp, int target_id, const char *message);
char* model_read_messages(ShmBuf *shmp, int shell_id);


/* Function declarations for view */
void view_init(int num_shells, ShmBuf *shmp);
void view_update_terminal(ShellInstance *shell, const char *output);
void view_update_message_area(ShellInstance *shell, const char *message);
void view_cleanup(void);

/* Function declarations for controller */
void controller_init(ShmBuf *shmp);
void controller_process_input(ShellInstance *shell, const char *input, ShmBuf *shmp);
void controller_update_views(void);
void controller_cleanup(void);

/* Global variables */
extern ShellInstance shells[MAX_SHELLS];
extern int num_shells;
extern ShmBuf *shared_buffer;
extern int open_shell_windows;

// common.h'e ekle
extern size_t last_read_offset[MAX_SHELLS];

#endif /* COMMON_H */