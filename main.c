#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

// maximum character input per command line
// 8192 bits = 1024 chars
#define MAX_INPUT 8192
// maximum number of args
#define MAX_TOKENS 128

void shell_loop();
char *read_line();
char **parse_line(char *line);
int execute_command(char **args);
int external_command(char **args);

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
        printf("shell> ");
        fflush(stdout);

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

char *read_line() {
    // allocate buffer to store input
    char *buffer = malloc(MAX_INPUT);

    // check if malloc succeeds
    if (!buffer) {
        // print error
        fprintf(stderr, "Error: malloc failed in read_line(): ");
        perror(NULL);
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
            fprintf(stderr, "Error: read failed in read_line(): ");
            perror(NULL);
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
        fprintf(stderr, "Error: read failed in parse_line(): ");
        perror(NULL);
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
    if (strcmp(args[0], "exit") == 0)
        return 0;

    // cd
    if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL) {
            fprintf(stderr, "cd: expected argument\n");
        } else {
            // change directory system call
            if (chdir(args[1]) != 0) {
                fprintf(stderr, "Error: chdir failed in execute_command(): ");
                perror(NULL);
            }
        }
        return 1;
    }

    return external_command(args);
}

int external_command(char **args) {
    pid_t pid = fork();
    // entry for child
    if (pid == 0) {
        // append args[0] to /user/bin/(command)
        char path[1024];
        snprintf(path, sizeof(path), "/usr/bin/%s", args[0]);
        // run child process
        // without searching PATH environment variables
        execv(path, args);
        fprintf(stderr, "Error: execv failed in external_command(): ");
        perror(NULL);
        exit(EXIT_FAILURE);
    }
    // fork error for parent
    if (pid < 0) {
        fprintf(stderr, "Error: fork failed in external_command(): ");
        perror(NULL);
        return 0;
    }
    // parent waits for child
    if (pid > 0) {
        waitpid(pid, NULL, 0);
    }

    return 1;
}