#ifndef VIEW_H
#define VIEW_H

#include <gtk/gtk.h>
extern GtkWidget *grid_gui;
extern GtkWidget *window;
void update_button_label(GtkButton *button, int row, int col);
GtkWidget* create_main_window(GtkApplication *app);
extern void on_cell_clicked(GtkButton *button, gpointer data);
#endif