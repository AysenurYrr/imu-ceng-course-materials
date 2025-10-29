#include "common.h"

/* Timer ID for periodic updates */
static guint update_timer_id = 0;

// controller.c içinde, en üstte:
size_t last_read_offset[MAX_SHELLS] = {0};


/* Callback for periodic updates */
static gboolean update_callback(gpointer user_data) {
    controller_update_views();
    return G_SOURCE_CONTINUE;  /* Continue calling */
}

/* Initialize the controller */
void controller_init(ShmBuf *shmp) {
    update_timer_id = g_timeout_add(500, update_callback, NULL);
}

/* Check if input is a message to everyone or specific shell */
bool is_message(const char *input) {
    return strncmp(input, "@msg", 4) == 0;
}

/* Parse @msg or @msgX format */
bool parse_message_command(const char *input, int *target_shell_id, const char **message_body) {
    if (strncmp(input, "@msg", 4) != 0)
        return false;

    if (input[4] == ' ') {
        *target_shell_id = 0;
        *message_body = input + 5;
        return true;
    }

    if (isdigit(input[4])) {
        int id = atoi(&input[4]);
        char *space = strchr(input, ' ');
        if (space && id >= 1 && id <= MAX_SHELLS) {
            *target_shell_id = id;
            *message_body = space + 1;
            return true;
        }
    }

    return false;
}

/* Process user input */
void controller_process_input(ShellInstance *shell, const char *input, ShmBuf *shmp) {
    if (!shell || !input || !shmp || strlen(input) == 0) {
        return;
    }

    int target_shell;
    const char *message_body;

    if (parse_message_command(input, &target_shell, &message_body)) {
        char formatted_message[BUF_SIZE];
        snprintf(formatted_message, BUF_SIZE, "Shell %d: %s", shell->id + 1, message_body);

        shmp->shell = target_shell;  // 0 for broadcast, X for specific shell
        model_send_message(shmp, formatted_message);

        char confirmation[BUF_SIZE];
        if (target_shell == 0) {
            snprintf(confirmation, BUF_SIZE, "[Gönderildi] %s\n", message_body);
        } else {
            snprintf(confirmation, BUF_SIZE, "[DM → Shell %d] %s\n", target_shell, message_body);
        }
        view_update_terminal(shell, confirmation);
    } else {
        char command_echo[BUF_SIZE];
        snprintf(command_echo, BUF_SIZE, "$ %s\n", input);
        view_update_terminal(shell, command_echo);

        model_execute_command(shell, input);
    }
}

/* Update all views with latest data */
void controller_update_views(void) {
    for (int i = 0; i < num_shells; i++) {
        char *messages = model_read_messages(shared_buffer, shells[i].id);
        if (messages && strlen(messages) > 0) {
            view_update_message_area(&shells[i], messages);
        }
    }
}

/* Clean up controller resources */
void controller_cleanup(void) {
    if (update_timer_id > 0) {
        g_source_remove(update_timer_id);
        update_timer_id = 0;
    }
}
