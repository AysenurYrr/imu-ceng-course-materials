/**
 * @file controller.c
 * @author your name (you@domain.com)
 * @brief
 * - Handles the button click event.
 * - When a button is clicked,
 *  - it gets the position,
 *  - prompts for input, updates the model,
 *  - and then updates the view.
 * @version 0.1
 * @date 2025-02-14
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "controller.h"
#include "model.h"
#include "view.h"

#include <stdlib.h>

/**
 * @brief 
 * 
 * @param button 
 * @param data 
 */
void reveal_cells(int row, int col) {
    if (row < 0 || row >= SIZE || col < 0 || col >= SIZE) return;
    if (grid[row][col] < 0) return; // Mine
    
    GtkWidget *button = gtk_grid_get_child_at(GTK_GRID(grid_gui), col, row);
    if (!GTK_IS_BUTTON(button)) return;
    
    // If already revealed
    const char *current_label = gtk_button_get_label(GTK_BUTTON(button));
    if (current_label[0] != 'X') return;
    
    update_button_label(GTK_BUTTON(button), row, col);
    
    // If empty cell, reveal neighbors
    if (grid[row][col] == 0) {
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                if (i == 0 && j == 0) continue;
                reveal_cells(row + i, col + j);
            }
        }
    }
}

void on_cell_clicked(GtkButton *button, gpointer data) {
    int *pos = (int *)data;
    int row = pos[0], col = pos[1];

    if (mfield[row][col] == 1) { // Hit a mine
        gameover = 1;
        gtk_button_set_label(button, "💣");
        gtk_widget_set_sensitive(grid_gui, FALSE);
    } else {
        reveal_cells(row, col);
    }
}