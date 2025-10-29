[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/ZjRHguIV)
**Lab Document: System Programming**  
**Lab Session 1: Setting Up the Environment and Basic Unix Commands**  

**this document is written with the help of DeepSeek.**

---

### **Lab Objectives**  
1. Install and configure a Unix environment using **WSL (Windows Subsystem for Linux)** or a **Virtual Machine (VM)**.  
2. Learn basic Unix terminal commands.  
3. Write and execute simple Bash scripts.  

---

### **Prerequisites**  
- A computer with **Windows 10/11** (for WSL) or any OS (for VMs like VirtualBox).  
- Internet access for downloading software.  
- Basic familiarity with command-line interfaces (optional).  

---

## **Part 1: Setting Up the Unix Environment**  

**If you have mac or Linux machine you can directly use them.** 

You can also use a virtual machine or  a docker instance. 
Below is the steps for WSL for windows and virtual machine for all OS.

### **Option 1: Install WSL (Windows Users)**  
1. **Enable WSL**:  
   Open PowerShell as **Administrator** and run:  
   ```powershell  
   wsl --install  
   ```  
   This installs WSL 2 and the default Ubuntu distribution.  

2. **Reboot your system** when prompted.  

3. **Launch Ubuntu**:  
   Search for "Ubuntu" in the Start menu and open it.  
   - Create a Unix username and password when prompted.  

4. **Update packages**:  
   ```bash  
   sudo apt update && sudo apt upgrade -y  
   sudo apt install build-essential
   ```  
5. You can setup vscode to use WSL (at the bottom left corner there is a WSL green area)
   
### **Option 2: Virtual Machine (All OS Users)**  
1. Install **VirtualBox** from [virtualbox.org](https://www.virtualbox.org/). 
   1. For mac use [utm macOS](https://mac.getutm.app/) or [utm iOS](https://getutm.app/) 
2. Download an **Ubuntu ISO** from [ubuntu.com](https://ubuntu.com/download/desktop). 
   1. for mac M chips, downlod ARM versions. 
3. Create a new VM in VirtualBox and follow the setup wizard.  

---

## **Part 2: Basic Unix Terminal Commands**  

### **1. Navigating the Filesystem**  
- `pwd`: Print current directory.  
- `ls`: List directory contents.  
- `cd <directory>`: Change directory.  
- `mkdir <dir_name>`: Create a directory.  
- `touch <file.txt>`: Create an empty file.  

**Try**:  
```bash  
mkdir lab1  
cd lab1  
touch hello.txt  
```  

### **2. File Operations**  
- `cp <source> <destination>`: Copy a file.  
- `mv <source> <destination>`: Move or rename a file.  
- `rm <file>`: Delete a file (use `-r` for directories).  

**Example**:  
```bash  
cp hello.txt hello_backup.txt  
mv hello.txt greeting.txt  
```  

### **3. Viewing Files**  
- `cat <file>`: Display file content.  
- `more <file>` or `less <file>`: Scroll through large files.  
- `head -n 5 <file>`: Show first 5 lines.  
- `tail -n 5 <file>`: Show last 5 lines.  

### **4. Permissions**  
- `chmod +x <file>`: Make a file executable.  
- `sudo <command>`: Run a command as superuser.  

---

## **Part 3: Bash Scripting Basics**  

### **1. Create a Simple Script**  
1. Write a script using `nano` or `vim`:  
   ```bash  
   nano hello.sh  
   ```  
2. Add the following code:  
   ```bash  
   #!/bin/bash  
   echo "Hello, World!"  
   ```  
3. Save and exit (`Ctrl+O`, `Ctrl+X` in Nano).  
4. Make it executable and run:  
   ```bash  
   chmod +x hello.sh  
   ./hello.sh  
   ```  

### **2. Variables and User Input**  
```bash  
#!/bin/bash  
echo "What's your name?"  
read name  
echo "Welcome, $name!"  
```  

### **3. Conditional Statements**  
```bash  
#!/bin/bash  
if [ $1 -gt 10 ]; then  
  echo "$1 is greater than 10."  
else  
  echo "$1 is less than or equal to 10."  
fi  
```  

### **4. Loops**  
**For Loop**:  
```bash  
for i in {1..5}; do  
  echo "Count: $i"  
done  
```  

**While Loop**:  
```bash  
counter=1  
while [ $counter -le 5 ]; do  
  echo "Counter: $counter"  
  ((counter++))  
done  
```  

---

## **Part 4: Examples**  

### **Example 1: System Info Script**  
```bash  
#!/bin/bash  
echo "System Report:"  
echo "Kernel: $(uname -r)"  
echo "User: $(whoami)"  
echo "Date: $(date)"  
```  

### **Example 2: Backup Script**  
```bash  
#!/bin/bash  
tar -czvf backup_$(date +%Y%m%d).tar.gz /path/to/directory  
echo "Backup created!"  
```  

---

## **Exercises**  
1. Write bash scrit `system_report.sh` that :  
   - Greets the user with `"hello"` 
   - Lists all files in the home directory.  
   - Creates a timestamped log file in that directory: `system_report_timestamp.log`  
2. Write a script (`even_odd.sh`) that accepts a number as input and prints whether it’s even or odd.  
3. Modify the backup script as `custom_backup.sh` to accept a directory path as an argument.  

---

## **Conclusion**  
By the end of this lab, students will:  
- Have a working Unix environment (WSL/VM).  
- Understand basic terminal commands.  
- Be able to write and execute simple Bash scripts.  

---

**Additional Resources**:  
- [WSL Documentation](https://docs.microsoft.com/en-us/windows/wsl/)  
- [Bash Scripting Guide](https://www.gnu.org/software/bash/manual/)  
- Use `man <command>` (e.g., `man ls`) to explore command manuals.  

---


**Answer Key**:  
Sample solutions for exercises will be provided in the next lab session.  You can also take help from AI assistant.

