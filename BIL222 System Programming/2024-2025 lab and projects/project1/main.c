#include "common.h"

/* Global variables */
GtkWidget *start_window;
GtkWidget *spin_button;
ShmBuf *global_shmp = NULL;
int open_shell_windows = 0;

/* Callback for Start button */
void on_start_button_clicked(GtkButton *button, gpointer user_data) {
    int shell_count = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_button));

    gtk_widget_destroy(start_window);  // Launch screen kapatılır

    /* Shell pencereleri başlatılır */
    view_init(shell_count, global_shmp);
    controller_init(global_shmp);
}

/* Başlangıç ekranını başlat */
void show_startup_window() {
    start_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(start_window), "Kabuk Sayısı Seç");
    gtk_window_set_default_size(GTK_WINDOW(start_window), 300, 120);
    gtk_container_set_border_width(GTK_CONTAINER(start_window), 10);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(start_window), vbox);

    GtkWidget *label = gtk_label_new("Kaç adet kabuk açmak istiyorsunuz?");
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);

    spin_button = gtk_spin_button_new_with_range(1, MAX_SHELLS, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_button), 2);  // varsayılan
    gtk_box_pack_start(GTK_BOX(vbox), spin_button, FALSE, FALSE, 0);

    GtkWidget *start_btn = gtk_button_new_with_label("Başlat");
    gtk_box_pack_start(GTK_BOX(vbox), start_btn, FALSE, FALSE, 0);

    g_signal_connect(start_btn, "clicked", G_CALLBACK(on_start_button_clicked), NULL);

    gtk_widget_show_all(start_window);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    /* Shared memory başlatılır */
    global_shmp = model_init();
    if (!global_shmp) {
        fprintf(stderr, "Paylaşımlı bellek başlatılamadı.\n");
        return EXIT_FAILURE;
    }

    /* Başlangıç ekranını göster */
    show_startup_window();

    /* GTK ana döngüsü */
    gtk_main();

    /* Temizlik */
    controller_cleanup();
    view_cleanup();
    model_cleanup(global_shmp);

    return EXIT_SUCCESS;
}
