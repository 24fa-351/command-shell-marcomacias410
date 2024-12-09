#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_LINE 1000

#include "parse.h"
#include "func_commands.h"

#define READ_SIDE 0
#define WRITE_SIDE 1


void remove_newline(char* line) {
    size_t len = strlen(line);
    if (len > 0 && line[len - 1] == '\n') {
        line[len - 1] = '\0';
    }
}

int main(int argc, char* argv[]) {
    env_vars_t env_vars[1000];

    char line[MAX_LINE];
    char* words[1000];
    char absolute_path[1000];

    while (1) {
        printf("myshell> ");
        fgets(line, MAX_LINE, stdin);

        remove_newline(line);

        if (!strcmp(line, "exit")) {
            break;
        }

        split(line, words, ' ');

        if (handle_internals(words, line, env_vars) == -1) {
            continue;
        }

        if (!find_absolute_path(words[0], absolute_path)) {
            printf("Command not found: %s\n", words[0]);
            continue;
        }

        int fd_input = STDIN_FILENO;
        int fd_output = STDOUT_FILENO;

        if (find_pipe(words) != -1) {
            handle_pipe(words, sizeof(words));
        } else {
        if (handle_redir(words, absolute_path) != -1) {
            
        }
        continue;
        }
    }

    return 0;
}