# Simple Unix Shell

This project is a minimal command-line shell in C  
It supports running basic Unix commands using POSIX system calls such as `fork()`, `execvp()`, `waitpid()`, `read()`, `chdir()`, `pipe()` and `dup2()`

The shell was created to help me learn about Unix processes, system calls, basic command parsing and inter-process communication

---

## Features

- Supports built-in commands:
  - `cd` — change directory
  - `exit` — quit shell
- Runs Unix commands by creating child processes
- Supports simple two-command pipelines

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