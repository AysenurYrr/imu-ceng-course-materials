[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/1kOymV5Q)
# Lab Session: Creating a Simple Shell Program
---

## **Lab Objective**  
Reinforce understanding of file I/O system calls (`open`, `read`, `write`, `close`) and process creation (`fork`, `exec`).  
**Tools**: C programming, `strace` for system call tracing, terminal.

---

## **Part-1: Basic File I/O**  
**Use only `open`, `read`, `write`, `close`;** No `printf`, `fopen` etc. 
**Task-1**: 
1. write a program that reads 
user input and outputs it.
1. Modify your program to save it to a file whose name is given by user.

**Task-2**: 
1. Write a program that runs forever, gets user input and prints to the stdout.
   1. Do this by reading/different number of bytes and buffers.
   2. What could be the complications your program would encounter.
      1. Ask AI to adjust your buffer size. 
2. Modify your program to get user input and print word by word.
   1. Use `strtok_r()` or `strsep()`
3. Modify your program so that when the user enters `cat`, it waits for the user input. 
And prints the user input. It repeats this till `EOF`.
1. Modify your program so that when user inputs `cat > newfile.txt`,
instead of printing to the stdout, it writes to the file.
 
**Task-3**: 
1. Write a program to copy a file (`source.txt` ➔ `dest.txt`) using system calls.  

    **Some useful steps:**
    1. Open `source.txt` in read-only mode (`O_RDONLY`).  
    4. Create/open `dest.txt` in write mode (`O_WRONLY | O_CREAT`), set permissions to `0644`.  
    5. Read `source.txt` in chunks (e.g., 1024 bytes) and write to `dest.txt`.  
    6. Handle errors for all system calls (use `perror`).  
   
2. Modify your program by integrating **task-2** so that 
   - when user enters `cat file1.txt file2.txt > newfile.txt`, 
   it copies the content of the input files to the new file.



**Test**:  
- Verify `dest.txt` matches `source.txt` using `diff source.txt dest.txt`.

---

### Optional Extra
This part is optional you can skip to part-2

**(Optional) Task:** a GUI-program `diff` that shows diff of two files as in git.

**Step-1**: Write a program to compare two files and print line-by-line differences.  
Steps:  
1. Open both files using `open`.  
2. Read lines from each file into dynamically allocated arrays.  
3. Compare lines:  
   - Identify added, removed, or modified lines.  
4. Print differences to the console (similar to `git diff`).  

**Test**:  
- Create test files (`file1.txt` and `file2.txt`) with minor differences.  
- Run the program and verify output matches expectations.  

**Optional Task: GTK GUI for Differences**  
Create a GTK application to display two files side-by-side with differences highlighted.  

**Steps**: 
1. Install GTK development libraries:  
   ```bash
   sudo apt-get install libgtk-4-dev  # For Debian/Ubuntu
   ```
   **Compilation**:  
    ```bash
    gcc `pkg-config --cflags gtk4`  main.c `pkg-config --libs gtk4`
    ```
2. Use `GtkTextView` widgets to display file contents.  Example steps;
   1. create two or three text views
   2. add them to a grid
   3. update their textbuffer by using text_buffer_set_text...
3. Highlight differing lines using background colors.  

---

## **Part-2: Shell with Fork & Exec**  
You can only use `exec` family system calls.
**Task-1**: Write a program that executes a given user command. E.g a single `ls`

**Task-2: Adding arguments**: 
- Modify your program so that it tokenizes the user input word by word
   and runs them. 
   - Make your program modular by creating functions for different tasks.
  - create a function `execute_command(struct command *cmd)` that runs a command. 
  - create a function to tokenize the given string.
  - In the `main()` adjust the arguments for the function and call them.

**Task-3**: Integrate the program in part-1 to this program.
- Add redirections etc. from part-1

**Task-4**:
Write a program where a parent process creates a child process.  
- The child executes the `ls -l` command using `execvp`.  
**Steps:**
1. Use `fork()` to create a child process.  
2. In the child:  
   - Build arguments for `execvp` (e.g., `{"ls", "-l", NULL}`).  
   - Call `execvp("ls", args)`.  

**Task-5** Modify your program so that the parent waits for the child to finish using `waitpid`.
- In the parent:  
   - Use `waitpid()` to wait for the child.  
   - Print the child’s exit status.  

**Test**:  
- Run the program. Verify `ls -l` output is displayed, and the parent prints the exit status.


## **Part 3:  Final Shell**  
**Task**: Create a program that mimics a basic shell: Integrate your shell from part-1 into part-2
1. Prompt the user for a command (e.g., `cat file.txt`).  
2. Fork a child process to execute the command.  
3. Wait for the child to complete.  


---

## **Debugging with `strace`**  
**Task**: Trace system calls made by your programs.  
1. Compile your code: `gcc -o myprogram mycode.c`.  
2. Run with `strace`:  
   ```bash
   strace -o trace.txt ./myprogram
   ```  
3. Open `trace.txt` to analyze system calls like `open`, `read`, `fork`, `execve`.

---

## **Submission**  
1. Code for all three exercises.  
2. Screenshots of outputs.  
3. `strace` output.  

---

## **Grading Criteria**  
- Correct use of system calls.  
- Error handling for all system calls.  
- Proper process management (waiting for children).  
- Programming standards: Code documentation (comments), indentation etc.  

