#include "model.h"
#include <stdio.h>
#include <stdlib.h>

/*1:indicates a mine in field[i][j]*/
int mfield[SIZE][SIZE] = {'\0'};


/*grid[i][j] indicates number of mines 
this should be calculated from mfield*/
int grid[SIZE][SIZE] = {'\0'};

/**
 * @brief 
 * 
 * @param filename 
 */
void load_mfield(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) return;
    
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            fscanf(file, "%d,", &mfield[i][j]);
        }
    }
    fclose(file);
    init_grid();
}

/**
 * @brief 
 * 
 * @param mine_percentage 
 */
void generate_random_mfield(int mine_percentage) {
    int total_cells = SIZE * SIZE;
    int num_mines = (total_cells * mine_percentage) / 100;
    
    // Clear field first
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            mfield[i][j] = 0;
        }
    }
    
    // Place mines randomly
    while (num_mines > 0) {
        int row = rand() % SIZE;
        int col = rand() % SIZE;
        if (mfield[row][col] != 1) {
            mfield[row][col] = 1;
            num_mines--;
        }
    }
    init_grid();
}

/**
 * @brief 
 * 
 * @param row 
 * @param col 
 * @return int 
 */
int num_of_surrounding_mines(int row, int col) {
    int count = 0;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            int new_row = row + i;
            int new_col = col + j;
            if (new_row >= 0 && new_row < SIZE && 
                new_col >= 0 && new_col < SIZE) {
                count += mfield[new_row][new_col];
            }
        }
    }
    return count - mfield[row][col]; // Subtract center cell if it's a mine
}

/**
 * @brief 
 * 
 * @return int 
 */
int init_grid() {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (mfield[i][j] == 1) {
                grid[i][j] = -1; // Mark mines with -1
            } else {
                grid[i][j] = num_of_surrounding_mines(i, j);
            }
        }
    }
    return 0;
}
