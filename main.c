#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

// maximum character input per command line
// 2048 bytes
#define MAX_INPUT 2048
// maximum number of args
#define MAX_TOKENS 128

void shell_loop();
void print_prompt();
char *read_line();
char **parse_line(char *line);
int execute_command(char **args);
int external_command(char **args);
int execute_pipe(char **args);

int main() {
    shell_loop();
    return 0;
}

void shell_loop() {
    // pointer to first char of zero terminated string
    char *line;
    // pointer to array of strings (char*)
    char **args;
    // output from executing command
    int status = 1;

    while (status) {
        print_prompt();

        // read
        line = read_line();
        // tokenise
        args = parse_line(line);
        // run
        status = execute_command(args);

        free(line);
        free(args);
    }
}

void print_prompt() {
    char cwd[1024];

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s> ", cwd);
    } else {
        perror("Error in getcwd()");
        printf("?> ");
    }

    fflush(stdout);
}

char *read_line() {
    // allocate buffer to store input
    char *buffer = malloc(MAX_INPUT);

    // check if malloc succeeds
    if (!buffer) {
        perror("Error in malloc()");
        exit(EXIT_FAILURE);
    }

    int pos = 0;

    // loop through each char
    while (1) {
        char c;
        // read syscall on stdin (file num 0)
        ssize_t bytes_read = read(0, &c, 1);

        // read returns 0 means EOF (ctrl+d)
        if (bytes_read == 0) {
            printf("\n");
            exit(0);
        }

        // read returns error
        if (bytes_read < 0) {
            perror("Error in read()");
            exit(EXIT_FAILURE);
        }

        // if newline is read, terminate string and return buffer
        if (c == '\n') {
            buffer[pos] = '\0';
            return buffer;
        }

        // store character, increment pos
        buffer[pos++] = c;

        // prevent buffer overflow
        if (pos >= MAX_INPUT - 1) {
            buffer[pos] = '\0';
            return buffer;
        }
    }
}

char **parse_line(char *line) {
    char **tokens = malloc(MAX_TOKENS * sizeof(char *));
    char *token;
    int position = 0;

    if (!tokens) {
        perror("Error in malloc()");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, " \n");
    while (token != NULL && position < MAX_TOKENS - 1) {
        tokens[position++] = token;
        token = strtok(NULL, " \n");
    }

    tokens[position] = NULL;
    return tokens;
}

// executes built in commands (doesnt fork)
// return 0 = exit
int execute_command(char **args) {
    if (args[0] == NULL) {
        return 1;
    }

    // exit
    if (strcmp(args[0], "exit") == 0) {
        return 0;
    }

    // cd
    if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL) {
            // no argument: go to root directory
            if (chdir("/") != 0) {
                perror("Error in chdir()");
            }
        } else {
            // change to specified directory
            if (chdir(args[1]) != 0) {
                perror("Error in chdir()");
            }
        }
        return 1;
    }

    // pipe
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "|") == 0) {
            return execute_pipe(args);
        }
    }

    return external_command(args);
}

int external_command(char **args) {
    pid_t pid = fork();
    // entry for child
    if (pid == 0) {
        // run child process
        // searches using PATH environment variables
        execvp(args[0], args);
        perror("Error in execvp()");
        exit(EXIT_FAILURE);
    }
    // fork error for parent
    if (pid < 0) {
        perror("Error in fork()");
        return 0;
    }
    // parent waits for child
    if (pid > 0) {
        waitpid(pid, NULL, 0);
    }

    return 1;
}

// handle a single pipe: cmd1 | cmd2
int execute_pipe(char **args) {
    int pipe_pos = -1;

    // find |
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "|") == 0) {
            pipe_pos = i;
            break;
        }
    }

    // shouldnt occur
    if (pipe_pos == -1) {
        fprintf(stderr, "Pipe error: '|' not found\n");
        return 1;
    }

    // split command into two arrays
    args[pipe_pos] = NULL;       // terminate left command
    char **left_cmd = args;
    char **right_cmd = &args[pipe_pos + 1];

    int fd[2];
    if (pipe(fd) < 0) {
        perror("pipe");
        return 1;
    }

    // fork child 1 (left cmd)
    pid_t pid1 = fork();
    if (pid1 == 0) {
        // close read end
        close(fd[0]);
        // write to stdout
        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]);

        // run first program
        execvp(left_cmd[0], left_cmd);
        perror("execvp");
        exit(EXIT_FAILURE);
    }

    // fork child 2 (right cmd)
    pid_t pid2 = fork();
    if (pid2 == 0) {
        // close write end
        close(fd[1]);
        // read from stdin
        dup2(fd[0], STDIN_FILENO);
        close(fd[0]);

        // run second program
        execvp(right_cmd[0], right_cmd);
        perror("execvp");
        exit(EXIT_FAILURE);
    }

    // parent closes both ends of pipe
    close(fd[0]);
    close(fd[1]);

    // wait for both children
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);

    return 1;
}