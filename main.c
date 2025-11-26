#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// maximum character input per command line
// 8192 bits = 1024 chars
#define MAX_INPUT 8192
// maximum number of args
#define MAX_TOKENS 128

void shell_loop();
char *read_line();
char **parse_line(char *line);

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

        line = read_line();
        args = parse_line(line);
        // status = execute_command(args); run command

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
        perror("malloc");
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