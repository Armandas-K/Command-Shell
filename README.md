# Simple Unix Shell

This project is a **minimal command-line shell** implemented in C.  
It supports running basic Unix commands using system calls such as `fork()`, `execv()`, `waitpid()`, `read()`, and `chdir()`.

The shell was created to help me learn about **Unix process management**, **system calls**, and **basic command parsing**.

---

## Features

- Runs external commands located in `/usr/bin`
- Supports built-in commands:
    - `cd` — change directory
    - `exit` — quit the shell
- Reads input using the low-level `read()` system call
- Tokenises commands using `strtok()`
- Executes commands using `fork()` + `execv()`

---

## Compile

To compile the project:

```bash
gcc -o shell main.c
```

Run with:

```bash
./shell
```