#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void shell_loop();


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

        // line = read_line();             input
        // args = parse_line(line);        tokenise
        // status = execute_command(args); run command

        free(line);
        free(args);
    }
}