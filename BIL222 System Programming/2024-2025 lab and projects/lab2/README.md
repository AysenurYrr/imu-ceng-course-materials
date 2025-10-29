[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/098xL9xb)
**Lab Document: System Programming**  
**Lab Session 2: C arrays, file I/O, strings, intro to GUI**  

**this document is written with the help of DeepSeek.**

---

## **Lab objectives**  
1. Installing necessary libs and running GUI apps.  
2. Practicing C arrays, strings, file I/O.  
3. Writing and using Makefiles 

---

## **Prerequisites**  
- A Unix machine or **Windows 10/11**  with  WSL or any OS with Linux/Unix installed VMs.  
- Internet access for downloading software.  
- C programming basics.  

---

## Getting familiar with a few terms 
-  Ask AI the definitions/differeces of 
   -  directX, openGL, SDL2, unity, GTK, QT, Xlib, x11, xorg, Wayland 
   - e.g. Which of them can be used for GUI?
   - Which of them should be used for game development?
   - Which of them can be used with C?
   - Which of them can be used together? 

---

## **Part 1: Setting up the environment for GUI**  

**On Linux or WSL:**
```bash
sudo apt install libgtk-4-dev
```

**On macOS:**
first install [homebrew](https://brew.sh/):
```sh
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

Then using brew, install GTK:
```sh
brew install gtk4
```

---

### Testing things
Try to run the following code by using the following compile commands:  
**Compilation:**
```bash
gcc main.c -o main $(pkg-config --cflags --libs gtk4)
./main
```

**main.c for testing**
```C
#include <gtk/gtk.h>
static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Editor");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);
    gtk_window_present(GTK_WINDOW(window));
}
int main(int argc, char **argv) {
    GtkApplication *app = 
        gtk_application_new("org.lab.editor", 
            G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", 
        G_CALLBACK(activate), NULL);
    int status = 
        g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
```

---

## Part 2: Minesweeper implementation
[Model–view–controller (MVC)](https://en.wikipedia.org/wiki/Model%E2%80%93view%E2%80%93controller) is a software design pattern commonly used for developing user interfaces that divides the related program logic into three interconnected elements.

**MVC architecture separation:**

- **Model**: Manages grid data, mine generation, and validation logic.  
- **View**: Handles GUI rendering (GTK4) and updates.  
- **Controller**: Processes user input and coordinates Model/View interactions.  

---

### Provided files  
You will work with the following files:  
1. **`model.h`** & **`model.c`**: Minefield data and logic.  
2. **`view.h`** & **`view.c`**: GUI components and updates.  
3. **`controller.h`** & **`controller.c`**: User input handling.  
4. **`main.c`**: Application initialization and MVC setup.  

---

### Summary of missing parts for students to implement:
#### 1. Model Component (`model.c`)  
Implement all functions:  
- **`load_mfield()`**: Read mine data from a CSV file into `mfield`.  
- **`generate_random_mfield()`**: Generate mines randomly based on a percentage.  
- **`num_of_surrounding_mines()`**: Calculate adjacent mines for a cell.  
- **`init_grid()`**: Initialize the `grid` using `num_of_surrounding_mines`.  

#### 2. Controller Component (`controller.c`)  
Modify `on_cell_clicked` to:  
- **Recursively open surrounding cells** (if no mines nearby).  
- Update button labels via `update_button_label`.  

#### 3. Main Function (`main.c`)  
- Add a call to `generate_random_mfield` with a valid mine percentage in `main()`.  

#### 4. Makefile  
Create a Makefile with:  
- **`build`**: Compile the executable `msweeper`.  
- **`compile`**: Generate object files.  
- **`clean`**: Remove generated files.  
- *(Optional)* Add dependency installation (GTK4).  

**Example Compilation Command:**  
```bash  
gcc -o msweeper main.c model.c view.c controller.c $(pkg-config --cflags --libs gtk4)  
```  

---

## Submission Instructions  
1. Clone the repository to obtain the skeleton code.  
2. Implement the missing parts as specified.  
3. Ensure the Makefile works for `build`, `compile`, and `clean`.  
4. Test the game for correct mine propagation and GUI updates.  
5. Commit/push the files to github

---

## **Conclusion**  
By the end of this lab, students will:  
- Have a working minesweeper game.  
- Understand basic Makefile commands.  
- Be able to write and execute simple GTK applications.  

---

## **Additional resources**:  
- [GTK C introductory examples](https://docs.gtk.org/gtk4/getting_started.html)
- DeepSeek or anyother such as GPT, claude sonnet
  - You can try implementing this with `SDL`
    - the model remains the same
- Any online C programming tutorial  

---

## **Solution**:  
Sample solution will be provided in the next lab session.  You can also take help from AI assistant.

---

## File Contents (Do Not Modify Code Structure)  

### model.h
```c
#ifndef MODEL_H
#define MODEL_H
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#define SIZE 9

extern int grid[SIZE][SIZE];
extern int mfield[SIZE][SIZE];

void load_mfield(const char *filename);
void generate_random_mfield();
int init_grid();
#endif
```

### model.c
```c
#include "model.h"

/*1:indicates a mine in field[i][j]*/
int mfield[SIZE][SIZE] = {'\0'};


/*grid[i][j] indicates number of mines 
this should be calculated from mfield*/
int grid[SIZE][SIZE] = {'\0'};

void load_mfield(const char *filename) {
    /* TODO: 
        read a CVS file into mfield
    */
}

void generate_random_mfield(int mine_percentage) {
    /* TODO: 
        generate random mfield with given percentage of mines
    */
    

    /*call grid to initialize grid 
    which is used to count number of mines*/
    init_grid();
}

int num_of_surrounding_mines(int row, int col) {
    /* TODO: 
        find number of mines around mfield[row][col], 
        be careful with boundaries
    */
    return 0;
}

int init_grid() {
    /* TODO: 
        init grid by calling num_of_mines for each [row][col] 
    */
}

```

### view.h
```c
#ifndef VIEW_H
#define VIEW_H

#include <gtk/gtk.h>

void update_button_label(GtkButton *button, int row, int col);
GtkWidget* create_main_window(GtkApplication *app);

#endif
```

### view.c
You do not have to change anything, but you can make the view fancier..
```c
#include <stdio.h>
#include "view.h"
#include "model.h"

void update_button_label(GtkButton *button, int row, int col) {
    char label[2];
    snprintf(label, sizeof(label), "%d", grid[row][col]);
    gtk_button_set_label(button, label);
}

GtkWidget* create_main_window(GtkApplication *app) {
    window = gtk_application_window_new(app);
    grid_gui = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid_gui), 5);
    gtk_grid_set_row_spacing(GTK_GRID(grid_gui), 5);

    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {

            /* label is either X or a number that shows number of X */
            char *label;
            /*initial button labels*/
            //label = g_strdup_printf("%d", grid[i][j]);
            label = g_strdup("X");
            GtkWidget *button = gtk_button_new_with_label(label);
            g_free(label);

            int *pos = malloc(2 * sizeof(int));
            pos[0] = i; pos[1] = j;
            
            /*call on_cell_clicked, when i,j button is clicked*/
            g_signal_connect_data(button, "clicked", G_CALLBACK(on_cell_clicked), 
                                pos, (GClosureNotify)free, 0);
            
            /*attach button to the grid */
            gtk_grid_attach(GTK_GRID(grid_gui), button, j, i, 1, 1);
        }
    }

    gtk_window_set_child(GTK_WINDOW(window), grid_gui);
    gtk_window_set_title(GTK_WINDOW(window), "Mine Field");
    return window;
}
```

### controller.h
```c
#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <gtk/gtk.h>
int gameover = 0;
void on_cell_clicked(GtkButton *button, gpointer data);

#endif
```

### controller.c
```c
#include "controller.h"
#include "model.h"
#include "view.h"

#include <stdlib.h>

void on_cell_clicked(GtkButton *button, gpointer data) {
    int *pos = (int *)data;
    int row = pos[0], col = pos[1];

    if (grid[row][col] >= 0) { /*not a bomb*/
        /*TODO: update to open all cells that 
        needs to be recursively opened*/
        update_button_label(button, row, col);
    } else { /*if it is bomb!*/
        gameover = 1;
        /*disable buttons*/
        gtk_widget_set_sensitive(grid_gui, FALSE);
    }
}
```

### main.c
```c

#include <gtk/gtk.h>
#include "model.h"
#include "view.h"

static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window = create_main_window(app);
    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
    /* load or generate mfield */
    if (argc > 1) {
        load_mfield(argv[1]);
    } else {
        /*TODO:update generate_random_mfield 
        so it creates a field 
        with given percentage of bomb-cells */
        generate_random_mfield();
    }
    GtkApplication *app = gtk_application_new("org.example.minefield",
                            G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
```
