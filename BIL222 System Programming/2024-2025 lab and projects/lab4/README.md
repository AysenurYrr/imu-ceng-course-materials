[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/56fGB4oG)
# Lab Document: Debugging with GDB and Valgrind

## Prerequisites
- GCC, GDB, and Valgrind installed.
- Basic familiarity with terminal commands and C syntax.


## Objectives
1. Practice using GDB to debug segmentation faults and logical errors in a C program.
2. Use Valgrind to detect memory leaks and invalid memory operations in example program and an older version of a publicly available program.

---

## Part 1: Debugging with GDB

### Example Program (Buggy Code)
Create a file named `buggy.c` with the following code:

```c
#include <stdio.h>

void initialize(int *data) {
    *data = 100; // Dereferencing a NULL pointer
}

int main() {
    int *value = NULL;
    initialize(value); // Passing NULL to initialize()
    printf("Value: %d\n", *value);
    return 0;
}
```

### Tasks
1. **Compile with Debug Symbols**  
   ```bash
   gcc -g -o buggy buggy.c
   ```

2. **Run the Program**  
   Observe the segmentation fault:
   ```bash
   ./buggy
   ```

3. **Debug with GDB**  
   - Start GDB:
     ```bash
     gdb ./buggy
     ```
   - Run the program inside GDB:
     ```(gdb) run```
   - When the crash occurs, inspect the backtrace:
     ```(gdb) backtrace```
   - Identify the faulty line (in `initialize()`). Examine the value of `data`:
     ```(gdb) print data```
   - Set a breakpoint at `initialize()` to debug step-by-step:
     ```(gdb) break initialize```
     ```(gdb) run```
   - Step through the code using `next` and `step`.

4. **Fix the Bug**  
   - Correct the code by allocating memory for `value` before calling `initialize()`:
     ```c
     int main() {
         int *value = malloc(sizeof(int)); // Fix: Allocate memory
         initialize(value);
         printf("Value: %d\n", *value);
         free(value);
         return 0;
     }
     ```
5. **Analysis of struct sizes and memory alignment**
   - Create two types and two variables:
    ```c
    typedef struct{
        int a;
        int *pa;
        int b;
    } Foo1;

    typedef struct{
        int a;
        int b;
        int *pa;
    } Foo2;
    Foo2 f2;
    Foo1 f2;
    ```

   - Use GDB to print the sizes of types and variables:
    ```bash
    print sizeof(foo1)
    print sizeof(Foo1)
    print sizeof(foo2)
    print sizeof(Foo2)
    ```
   - Print also address of the variables and see the alignment
   - Add `__attribute__((packed, aligned(X)))` to the end of structs by using `1, 2, 4, 8, 16` values for `X` and repeat the printing.
  
---

## Part 2: Memory Analysis with Valgrind

### Settings
```bash
sudo apt update
sudo apt upgrade
sudo apt install valgrind
```

For starters, see [Valgrind Simple Example](https://valgrind.org/docs/manual/quick-start.html#quick-start.intro) also see alternatives e.g. [Code sanitizer](https://en.wikipedia.org/wiki/Code_sanitizer)


### Example Program (Memory Leaks + Invalid Access)
Create `leaky.c` with this code:

```c
#include <stdlib.h>
#include <stdio.h>

void leaky_function() {
    int* arr = malloc(sizeof(int) * 10); // Never freed
    arr[0] = 42;
}

void invalid_access() {
    int* ptr = malloc(sizeof(int));
    free(ptr);
    *ptr = 100; // Writing to freed memory!
}

int main() {
    leaky_function();
    invalid_access();
    return 0;
}
```

### Tasks
1. **Compile with Debug Symbols**
   ```bash
   gcc -g -o leaky leaky.c
   ```

2. **Run Under Valgrind**
   ```bash
   valgrind --leak-check=full ./leaky
   ```

3. **Analyze Output**  
   Valgrind will report:
   - **Memory leak** from `leaky_function()` (10 ints lost)
   - **Invalid write** in `invalid_access()` (writing to freed memory)

4. **Fix the Code**  
   Modified version:
   ```c
   void leaky_function() {
       int* arr = malloc(sizeof(int) * 10);
       arr[0] = 42;
       free(arr); // Fix leak
   }

   void invalid_access() {
       int* ptr = malloc(sizeof(int));
       *ptr = 100; // Write while memory is valid
       free(ptr);
   }
   ```

---

## Key Valgrind Output Patterns to Identify

### Memory Leak
```
==12345== 40 bytes in 1 blocks are definitely lost
==12345==    at 0x483B7F3: malloc (vg_replace_malloc.c:307)
==12345==    by 0x109186: leaky_function (leaky.c:4)
```

### Invalid Access
```
==12345== Invalid write of size 4
==12345==    at 0x1091D7: invalid_access (leaky.c:12)
==12345==    by 0x109206: main (leaky.c:16)
==12345==  Address 0x4a55040 is 0 bytes inside a block of size 4 free'd
```

---


## A more complex example
Find a publicly available program and analyze it by using valgrind (you can ask AI to find it).

---

## Part 3: Memory Analysis of Lab-3
Run valgrind on the code of lab-2 to determine if there is any memory leak.


## Submission
1. **GDB Section**  
   - Fixed `buggy.c` code.
   - Answers to:
     - What caused the segmentation fault?
     - How did GDB help identify the issue?

2. **Valgrind Section**  
   - Output from original Valgrind run showing both errors
   - Fixed version of `leaky.c` 
   - Summary of Valgrind findings (e.g., number of leaks, types of errors).
   - Answers to:
     - What does "definitely lost" mean in Valgrind's output?
     - Why is writing to freed memory dangerous even if the program doesn't crash?
     - Which function in publicly available program caused the largest memory leak?
     - How would you fix the most critical error reported?


---

## Questions for AI
1. **GDB**  
   - What command shows the line number where the program crashed?  
   - How do you inspect the value of a variable in a specific stack frame?

2. **Valgrind**  
   - What’s the difference between "definitely lost" and "indirectly lost" in Valgrind’s output?  
   - Why is it important to compile programs with `-g` when using Valgrind?

---
