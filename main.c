#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_INPUT 2048
#define MAX_TOKENS 128

void shell_loop();
void print_prompt();
char *read_line();
char **parse_line(char *line);
int execute_command(char **args);
int external_command(char **args);

int main() {
    shell_loop();
    return 0;
}

void shell_loop() {
    char *line;
    char **args;
    int status = 1;

    while (status) {
        print_prompt();

        line = read_line();
        args = parse_line(line);
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
    char *buffer = malloc(MAX_INPUT);

    if (!buffer) {
        perror("Error in malloc()");
        exit(EXIT_FAILURE);
    }

    int pos = 0;

    while (1) {
        char c;
        ssize_t bytes_read = read(0, &c, 1);

        if (bytes_read == 0) {
            printf("\n");
            exit(0);
        }

        if (bytes_read < 0) {
            perror("Error in read()");
            exit(EXIT_FAILURE);
        }

        if (c == '\n') {
            buffer[pos] = '\0';
            return buffer;
        }

        buffer[pos++] = c;

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

int execute_command(char **args) {
    if (args[0] == NULL) {
        return 1;
    }

    if (strcmp(args[0], "exit") == 0) {
        return 0;
    }

    if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL) {
            fprintf(stderr, "Error in cd: missing argument\n");
        } else {
            if (chdir(args[1]) != 0) {
                perror("Error in chdir()");
            }
        }
        return 1;
    }

    return external_command(args);
}

int external_command(char **args) {
    pid_t pid = fork();
    if (pid == 0) {
        char path[2048];
        snprintf(path, sizeof(path), "/usr/bin/%s", args[0]);
        args[0] = path;

        execv(path, args);
        perror("Error in execv()");
        exit(EXIT_FAILURE);
    }

    if (pid < 0) {
        perror("Error in fork()");
        return 0;
    }

    if (pid > 0) {
        waitpid(pid, NULL, 0);
    }

    return 1;
}