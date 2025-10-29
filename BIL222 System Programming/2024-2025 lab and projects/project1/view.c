#include "common.h"
#include <gdk/gdkkeysyms.h>

ShellInstance shells[MAX_SHELLS];
int num_shells = 0;
ShmBuf *shared_buffer = NULL;

/* Callback for command entry */
static void on_command_entered(GtkEntry *entry, gpointer user_data) {
    ShellInstance *shell = (ShellInstance *)user_data;
    const char *text = gtk_entry_get_text(GTK_ENTRY(entry));
    char cwd[1024];

    if (strlen(text) > 0) {
        if (shell->history_count < MAX_HISTORY) {
            shell->history[shell->history_count++] = g_strdup(text);
        }
        shell->history_index = shell->history_count;
    }
    
    controller_process_input(shell, text, shared_buffer);
    gtk_entry_set_text(GTK_ENTRY(entry), "");
    
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        char prompt[2048];
        snprintf(prompt, sizeof(prompt), "%s$ ", cwd);
        view_update_terminal(shell, prompt);

    }

}

/* Callback for key press (for command history) */
static gboolean on_entry_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data) {
    ShellInstance *shell = (ShellInstance *)user_data;
    if (event->keyval == GDK_KEY_Up) {
        if (shell->history_index > 0) {
            shell->history_index--;
            gtk_entry_set_text(GTK_ENTRY(shell->entry), shell->history[shell->history_index]);
        }
        return TRUE;
    } else if (event->keyval == GDK_KEY_Down) {
        if (shell->history_index < shell->history_count - 1) {
            shell->history_index++;
            gtk_entry_set_text(GTK_ENTRY(shell->entry), shell->history[shell->history_index]);
        } else {
            gtk_entry_set_text(GTK_ENTRY(shell->entry), "");
            shell->history_index = shell->history_count;
        }
        return TRUE;
    }

    return FALSE;
}

/* Callback for window close */
static gboolean on_window_close(GtkWidget *widget, GdkEvent *event, gpointer user_data) {
    open_shell_windows--;
    if (open_shell_windows <= 0) {
        gtk_main_quit();
    }
    return FALSE;
}

/* Initialize the view */
void view_init(int shell_count, ShmBuf *shmp) {
    shared_buffer = shmp;
    num_shells = shell_count > MAX_SHELLS ? MAX_SHELLS : shell_count;
    
    /* Create shell instances */
    for (int i = 0; i < num_shells; i++) {
        open_shell_windows++;
        shells[i].id = i;
        shells[i].process_count = 0;
        shells[i].history_index = 0;

        /* Create window */
        char window_title[32];
        sprintf(window_title, "Shell %d", i + 1);
        shells[i].window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title(GTK_WINDOW(shells[i].window), window_title);
        gtk_window_set_default_size(GTK_WINDOW(shells[i].window), 800, 600);
        g_signal_connect(shells[i].window, "delete-event", G_CALLBACK(on_window_close), NULL);
        
        /* Create layout */
        GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
        gtk_container_add(GTK_CONTAINER(shells[i].window), vbox);
        
        /* Create paned container */
        GtkWidget *paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
        gtk_box_pack_start(GTK_BOX(vbox), paned, TRUE, TRUE, 0);
        
        /* Create terminal area with terminal-like styling */
        GtkWidget *terminal_scroll = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(terminal_scroll),
                                      GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
        shells[i].terminal_view = gtk_text_view_new();
        shells[i].terminal_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(shells[i].terminal_view));
        gtk_text_view_set_editable(GTK_TEXT_VIEW(shells[i].terminal_view), FALSE);
        gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(shells[i].terminal_view), GTK_WRAP_WORD_CHAR);
        
        /* Set terminal-like colors */
        GdkRGBA bg_color;
        gdk_rgba_parse(&bg_color, "#2E3436");  /* Dark gray background */
        gtk_widget_override_background_color(shells[i].terminal_view, GTK_STATE_FLAG_NORMAL, &bg_color);
        
        /* Set text color */
        GdkRGBA text_color;
        gdk_rgba_parse(&text_color, "#EEEEEC");  /* Light gray text */
        gtk_widget_override_color(shells[i].terminal_view, GTK_STATE_FLAG_NORMAL, &text_color);
        
        /* Set monospace font */
        PangoFontDescription *font_desc = pango_font_description_from_string("Monospace 10");
        gtk_widget_override_font(shells[i].terminal_view, font_desc);
        pango_font_description_free(font_desc);
        
        /* Add padding */
        gtk_text_view_set_left_margin(GTK_TEXT_VIEW(shells[i].terminal_view), 8);
        gtk_text_view_set_right_margin(GTK_TEXT_VIEW(shells[i].terminal_view), 8);
        
        gtk_container_add(GTK_CONTAINER(terminal_scroll), shells[i].terminal_view);
        gtk_paned_add1(GTK_PANED(paned), terminal_scroll);
        
        /* Create message area with styling */
        GtkWidget *message_scroll = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(message_scroll),
                                      GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
        shells[i].message_view = gtk_text_view_new();
        shells[i].message_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(shells[i].message_view));
        gtk_text_view_set_editable(GTK_TEXT_VIEW(shells[i].message_view), FALSE);
        gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(shells[i].message_view), GTK_WRAP_WORD_CHAR);
        
        /* Set message area colors */
        GdkRGBA msg_bg_color;
        gdk_rgba_parse(&msg_bg_color, "#F9F9F9");  /* Light background */
        gtk_widget_override_background_color(shells[i].message_view, GTK_STATE_FLAG_NORMAL, &msg_bg_color);
        
        gtk_container_add(GTK_CONTAINER(message_scroll), shells[i].message_view);
        gtk_paned_add2(GTK_PANED(paned), message_scroll);
        
        /* Set initial position */
        gtk_paned_set_position(GTK_PANED(paned), 500);
        
        /* Create command entry with styling */
        shells[i].entry = gtk_entry_new();
        gtk_entry_set_placeholder_text(GTK_ENTRY(shells[i].entry), "Komut girin veya @msg ile mesaj gönderin...");
        
        /* Style the entry to look like a terminal prompt */
        GdkRGBA entry_bg_color;
        gdk_rgba_parse(&entry_bg_color, "#2E3436");  /* Match terminal background */
        gtk_widget_override_background_color(shells[i].entry, GTK_STATE_FLAG_NORMAL, &entry_bg_color);
        
        GdkRGBA entry_text_color;
        gdk_rgba_parse(&entry_text_color, "#EEEEEC");  /* Match terminal text */
        gtk_widget_override_color(shells[i].entry, GTK_STATE_FLAG_NORMAL, &entry_text_color);
        
        /* Set monospace font for entry */
        PangoFontDescription *entry_font_desc = pango_font_description_from_string("Monospace 10");
        gtk_widget_override_font(shells[i].entry, entry_font_desc);
        pango_font_description_free(entry_font_desc);
        
        gtk_box_pack_start(GTK_BOX(vbox), shells[i].entry, FALSE, FALSE, 0);
        g_signal_connect(shells[i].entry, "activate", G_CALLBACK(on_command_entered), &shells[i]);
        g_signal_connect(shells[i].entry, "key-press-event", G_CALLBACK(on_entry_key_press), &shells[i]);
        
        /* Create horizontal box for top labels */
        GtkWidget *label_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);

        /* Terminal label (left) */
        GtkWidget *terminal_label = gtk_label_new("Terminal Çıktısı");
        gtk_widget_set_halign(terminal_label, GTK_ALIGN_START);
        gtk_widget_set_hexpand(terminal_label, TRUE);
        gtk_box_pack_start(GTK_BOX(label_box), terminal_label, TRUE, TRUE, 0);

        /* Message label (right) */
        GtkWidget *message_label = gtk_label_new("Paylaşılan Mesajlar");
        gtk_widget_set_halign(message_label, GTK_ALIGN_END);
        gtk_widget_set_hexpand(message_label, TRUE);
        gtk_box_pack_start(GTK_BOX(label_box), message_label, TRUE, TRUE, 0);

        /* Add label box to the TOP of the vbox */
        gtk_box_pack_start(GTK_BOX(vbox), label_box, FALSE, FALSE, 0);
        gtk_box_reorder_child(GTK_BOX(vbox), label_box, 0);  // Move it to the top
        
        /* Show all widgets */
        gtk_widget_show_all(shells[i].window);
        
        /* Position windows */
        gtk_window_move(GTK_WINDOW(shells[i].window), 100 + i * 50, 100 + i * 50);
        
        /* Add welcome message */
        char welcome_msg[512];
        snprintf(welcome_msg, sizeof(welcome_msg),
                "╔══════════════════════════════════════════════════╗\n"
                "║               Kabuk %d'e Hoş Geldiniz              ║\n"
                "╚══════════════════════════════════════════════════╝\n\n"
                "Komutları doğrudan yazabilirsiniz (örn: ls, pwd, echo)\n"
                "Mesaj göndermek için: @msg Mesajınız\n\n"
                "$ ", i + 1);
        view_update_terminal(&shells[i], welcome_msg);
    }

    /* CSS ile stil uygulamak için */
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider,
        "textview.terminal { background-color: #2E3436; color: #EEEEEC; font-family: monospace; }"
        "entry.command { background-color: #2E3436; color: #EEEEEC; font-family: monospace; }",
        -1, NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    /* Sonra widget'a sınıf ekleyin */
    for (int i = 0; i < num_shells; i++) {
        GtkStyleContext *context = gtk_widget_get_style_context(shells[i].terminal_view);
        gtk_style_context_add_class(context, "terminal");
    }
}

/* Update terminal output with syntax highlighting */
void view_update_terminal(ShellInstance *shell, const char *output) {
    if (!shell || !output) {
        return;
    }
    
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(shell->terminal_buffer, &iter);
    
    /* Create text tags for syntax highlighting if they don't exist */
    GtkTextTag *command_tag = gtk_text_buffer_get_tag_table(shell->terminal_buffer) ?
                             gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(shell->terminal_buffer), "command") : NULL;
    
    if (!command_tag) {
        command_tag = gtk_text_buffer_create_tag(shell->terminal_buffer, "command", 
                                               "foreground", "#729FCF", /* Light blue */
                                               "weight", PANGO_WEIGHT_BOLD, 
                                               NULL);
    }
    
    GtkTextTag *prompt_tag = gtk_text_buffer_get_tag_table(shell->terminal_buffer) ?
                            gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(shell->terminal_buffer), "prompt") : NULL;
    
    if (!prompt_tag) {
        prompt_tag = gtk_text_buffer_create_tag(shell->terminal_buffer, "prompt", 
                                              "foreground", "#8AE234", /* Light green */
                                              "weight", PANGO_WEIGHT_BOLD, 
                                              NULL);
    }
    
    GtkTextTag *error_tag = gtk_text_buffer_get_tag_table(shell->terminal_buffer) ?
                           gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(shell->terminal_buffer), "error") : NULL;
    
    if (!error_tag) {
        error_tag = gtk_text_buffer_create_tag(shell->terminal_buffer, "error", 
                                             "foreground", "#EF2929", /* Light red */
                                             NULL);
    }
    
    /* Apply basic syntax highlighting */
    if (output[0] == '$' && output[1] == ' ') {
        /* Command prompt line */
        GtkTextMark *start_mark = gtk_text_buffer_create_mark(shell->terminal_buffer, NULL, &iter, TRUE);
        gtk_text_buffer_insert(shell->terminal_buffer, &iter, "$ ", 2);
        GtkTextIter start_iter;
        gtk_text_buffer_get_iter_at_mark(shell->terminal_buffer, &start_iter, start_mark);
        gtk_text_buffer_apply_tag(shell->terminal_buffer, prompt_tag, &start_iter, &iter);
        
        /* Command text */
        start_mark = gtk_text_buffer_create_mark(shell->terminal_buffer, NULL, &iter, TRUE);
        gtk_text_buffer_insert(shell->terminal_buffer, &iter, output + 2, -1);
        gtk_text_buffer_get_iter_at_mark(shell->terminal_buffer, &start_iter, start_mark);
        gtk_text_buffer_apply_tag(shell->terminal_buffer, command_tag, &start_iter, &iter);
        
        gtk_text_buffer_delete_mark(shell->terminal_buffer, start_mark);
    } else if (strstr(output, "error") || strstr(output, "Error") || strstr(output, "failed") || strstr(output, "Failed")) {
        /* Error messages */
        GtkTextMark *start_mark = gtk_text_buffer_create_mark(shell->terminal_buffer, NULL, &iter, TRUE);
        gtk_text_buffer_insert(shell->terminal_buffer, &iter, output, -1);
        GtkTextIter start_iter;
        gtk_text_buffer_get_iter_at_mark(shell->terminal_buffer, &start_iter, start_mark);
        gtk_text_buffer_apply_tag(shell->terminal_buffer, error_tag, &start_iter, &iter);
        gtk_text_buffer_delete_mark(shell->terminal_buffer, start_mark);
    } else {
        /* Regular output */
        gtk_text_buffer_insert(shell->terminal_buffer, &iter, output, -1);
    }
    
    /* Scroll to the end */
    gtk_text_buffer_get_end_iter(shell->terminal_buffer, &iter);
    GtkTextMark *mark = gtk_text_buffer_create_mark(shell->terminal_buffer, NULL, &iter, FALSE);
    gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(shell->terminal_view), mark, 0.0, TRUE, 0.0, 1.0);
    gtk_text_buffer_delete_mark(shell->terminal_buffer, mark);
}

/* Update message area with colored messages */
void view_update_message_area(ShellInstance *shell, const char *message) {
    if (!shell || !message) {
        return;
    }
    
    /* Clear existing content */
    gtk_text_buffer_set_text(shell->message_buffer, "", 0);
    
    /* Create text tags for message highlighting if they don't exist */
    GtkTextTag *timestamp_tag = gtk_text_buffer_get_tag_table(shell->message_buffer) ?
                               gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(shell->message_buffer), "timestamp") : NULL;
    
    if (!timestamp_tag) {
        timestamp_tag = gtk_text_buffer_create_tag(shell->message_buffer, "timestamp", 
                                                 "foreground", "#888A85", /* Gray */
                                                 "scale", PANGO_SCALE_SMALL,
                                                 NULL);
    }
    
    GtkTextTag *shell_tag = gtk_text_buffer_get_tag_table(shell->message_buffer) ?
                           gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(shell->message_buffer), "shell") : NULL;
    
    if (!shell_tag) {
        shell_tag = gtk_text_buffer_create_tag(shell->message_buffer, "shell", 
                                             "foreground", "#3465A4", /* Blue */
                                             "weight", PANGO_WEIGHT_BOLD,
                                             NULL);
    }
    
    GtkTextTag *message_tag = gtk_text_buffer_get_tag_table(shell->message_buffer) ?
                             gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(shell->message_buffer), "message_text") : NULL;
    
    if (!message_tag) {
        message_tag = gtk_text_buffer_create_tag(shell->message_buffer, "message_text", 
                                               "foreground", "#000000", /* Black */
                                               NULL);
    }
    
    /* Parse and add messages with highlighting */
    GtkTextIter iter;
    gtk_text_buffer_get_start_iter(shell->message_buffer, &iter);
    
    char *msg_copy = strdup(message);
    char *line = strtok(msg_copy, "\n");
    
    while (line != NULL) {
        if (strlen(line) > 0) {
            /* Find timestamp in square brackets */
            char *timestamp_end = strstr(line, "] ");
            if (timestamp_end) {
                size_t timestamp_len = (timestamp_end - line) + 1;
                
                /* Insert and tag timestamp */
                GtkTextMark *start_mark = gtk_text_buffer_create_mark(shell->message_buffer, NULL, &iter, TRUE);
                gtk_text_buffer_insert(shell->message_buffer, &iter, line, timestamp_len);
                GtkTextIter start_iter;
                gtk_text_buffer_get_iter_at_mark(shell->message_buffer, &start_iter, start_mark);
                gtk_text_buffer_apply_tag(shell->message_buffer, timestamp_tag, &start_iter, &iter);
                gtk_text_buffer_delete_mark(shell->message_buffer, start_mark);
                
                /* Find shell identifier */
                char *shell_end = strstr(line + timestamp_len, ": ");
                if (shell_end) {
                    size_t shell_len = (shell_end - (line + timestamp_len)) + 1;
                    
                    /* Insert and tag shell identifier */
                    start_mark = gtk_text_buffer_create_mark(shell->message_buffer, NULL, &iter, TRUE);
                    gtk_text_buffer_insert(shell->message_buffer, &iter, line + timestamp_len + 1, shell_len);
                    gtk_text_buffer_get_iter_at_mark(shell->message_buffer, &start_iter, start_mark);
                    gtk_text_buffer_apply_tag(shell->message_buffer, shell_tag, &start_iter, &iter);
                    gtk_text_buffer_delete_mark(shell->message_buffer, start_mark);
                    
                    /* Insert and tag message content */
                    start_mark = gtk_text_buffer_create_mark(shell->message_buffer, NULL, &iter, TRUE);
                    gtk_text_buffer_insert(shell->message_buffer, &iter, shell_end + 1, -1);
                    gtk_text_buffer_get_iter_at_mark(shell->message_buffer, &start_iter, start_mark);
                    gtk_text_buffer_apply_tag(shell->message_buffer, message_tag, &start_iter, &iter);
                    gtk_text_buffer_delete_mark(shell->message_buffer, start_mark);
                } else {
                    /* No shell identifier, just insert the rest */
                    gtk_text_buffer_insert(shell->message_buffer, &iter, line + timestamp_len + 1, -1);
                }
            } else {
                /* No timestamp format, just insert the line */
                gtk_text_buffer_insert(shell->message_buffer, &iter, line, -1);
            }
        }
        
        gtk_text_buffer_insert(shell->message_buffer, &iter, "\n", 1);
        line = strtok(NULL, "\n");
    }
    
    free(msg_copy);
    
    /* Scroll to the end */
    gtk_text_buffer_get_end_iter(shell->message_buffer, &iter);
    GtkTextMark *mark = gtk_text_buffer_create_mark(shell->message_buffer, NULL, &iter, FALSE);
    gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(shell->message_view), mark, 0.0, TRUE, 0.0, 1.0);
    gtk_text_buffer_delete_mark(shell->message_buffer, mark);
}

/* Clean up view resources */
void view_cleanup(void) {
    for (int i = 0; i < num_shells; i++) {
        gtk_widget_destroy(shells[i].window);
    }
} 