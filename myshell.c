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

/*



*****TODO: Implement the following functions in parse.c*****

copy the split words and move them into a new array
use the new array to use it in the execve function

shift the array left after the redirection symbol or output file is found



*/

// bool redirect_input(char* words[], int* fd_input) {
//     for (int ix = 0; words[ix] != NULL; ix++) {
//         if (!strcmp(words[ix], "<")) {
//             if (words[ix + 1] == NULL) {
//                 fprintf(stderr, "No input file specified\n");
//                 return false;
//             }

//             printf("Redirecting input from %s\n", words[ix + 1]);
//             *fd_input = open(words[ix + 1], O_RDONLY);
//             if (*fd_input == -1) {
//                 perror("Failed to open input file");
//                 return false;
//             }
//             words[ix] = "";
//             words[ix + 1] = "";
//         }
//     }
//     return true;
// }

// void count_specials(char* words[], int* num_pipes, int* num_redirs) {
//     *num_pipes = 0;
//     *num_redirs = 0;

//     for (int ix = 0; words[ix] != NULL; ix++) {
//         if (!strcmp(words[ix], "|")) {
//             (*num_pipes)++;
//         } else if (!strcmp(words[ix], ">") || !strcmp(words[ix], "<")) {
//             (*num_redirs)++;
//         }
//     }
// }

// int find_pipe(char* words[]) {
//     for (int ix = 0; words[ix] != NULL; ix++) {
//         if (!strcmp(words[ix], "|")) {
//             return ix;
//         }
//     }
//     return -1;
// }

// void copy_ptrs_from_to(char** to, char** from, int from_ix, int to_ix) {
//     for (int dest_ix = 0; dest_ix < to_ix - from_ix + 1; dest_ix++)
//         to[dest_ix] = from[from_ix + dest_ix];
//     to[to_ix - from_ix + 1] = NULL;
// }

void remove_newline(char* line) {
    size_t len = strlen(line);
    if (len > 0 && line[len - 1] == '\n') {
        line[len - 1] = '\0';
    }
}

// void shift_array(char* words[], int start) {
//     for (int ix = start; words[ix] != NULL; ix++) {
//         words[ix] = words[ix + 1];
//     }
// }

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