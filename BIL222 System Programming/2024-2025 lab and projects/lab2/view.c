/**
 * @file view.c
 * @author your name (you@domain.com)
 * @brief
 * - Creates the window, grid, buttons.
 * - update_button function to change the label of a button.
 * @version 0.1
 * @date 2025-02-14
 *
 * @copyright Copyright (c) 2025
 *
 */
#include "view.h"
#include <stdio.h>
#include "model.h"

GtkWidget *grid_gui;  // Gerçek tanımlama burada
GtkWidget *window;    // Gerçek tanımlama burada

/**
 * @brief 
 * 
 */
void update_button_label(GtkButton *button, int row, int col) {
    char label[2];
    snprintf(label, sizeof(label), "%d", grid[row][col]);
    gtk_button_set_label(button, label);
}

/**
 * @brief Create a main window object
 * 
 * @param app 
 * @return GtkWidget* 
 */
GtkWidget *create_main_window(GtkApplication *app) {
    window = gtk_application_window_new(app);
    grid_gui = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid_gui), 5);
    gtk_grid_set_row_spacing(GTK_GRID(grid_gui), 5);

    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            /* label is either X or a number that shows number of X */
            char *label;
            /*initial button labels*/
            // label = g_strdup_printf("%d", grid[i][j]);
            label = g_strdup("X");
            GtkWidget *button = gtk_button_new_with_label(label);
            g_free(label);

            int *pos = malloc(2 * sizeof(int));
            pos[0] = i;
            pos[1] = j;

            /*call on_cell_clicked, when i,j button is clicked*/
            g_signal_connect_data(button, "clicked",
                                  G_CALLBACK(on_cell_clicked), pos,
                                  (GClosureNotify)free, 0);

            /*attach button to the grid */
            gtk_grid_attach(GTK_GRID(grid_gui), button, j, i, 1, 1);
        }
    }

    gtk_window_set_child(GTK_WINDOW(window), grid_gui);
    gtk_window_set_title(GTK_WINDOW(window), "Mine Field");
    return window;
}