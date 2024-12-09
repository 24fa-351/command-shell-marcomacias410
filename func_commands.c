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

#define READ_SIDE 0
#define WRITE_SIDE 1

int fd_input = STDIN_FILENO;
int fd_output = STDOUT_FILENO;

typedef struct env {
    char* key;
    char* value;
} env_vars_t;

int find_pipe(char* words[]) {
    for (int ix = 0; words[ix] != NULL; ix++) {
        if (!strcmp(words[ix], "|")) {
            return ix;
        }
    }
    return -1;
}

void copy_ptrs_from_to(char** to, char** from, int from_ix, int to_ix) {
    for (int dest_ix = 0; dest_ix < to_ix - from_ix + 1; dest_ix++)
        to[dest_ix] = from[from_ix + dest_ix];
    to[to_ix - from_ix + 1] = NULL;
}

int handle_internals(char* words[], char* line, env_vars_t env_vars[]) {
    if (!strcmp(words[0], "cd")) {
        if (chdir(words[1]) == -1) {
            perror("cd");
        }
        return -1;
    }

    if (!strcmp(words[0], "pwd")) {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s\n", cwd);
        } else {
            perror("getcwd");
        }
        return -1;
    }

    if (!strcmp(words[0], "echo")) {
        for (int ix = 1; words[ix] != NULL; ix++) {
            if (words[ix][0] == '$') {
                char* key = words[ix] + 1;
                char* value = NULL;

                for (int jx = 0; jx < 1000; jx++) {
                    if (env_vars[jx].key != NULL &&
                        !strcmp(env_vars[jx].key, key)) {
                        value = env_vars[jx].value;
                        break;
                    }
                }

                if (value != NULL) {
                    printf("%s ", value);
                }
            } else {
                printf("%s ", words[ix]);
            }
        }
        printf("\n");
        return -1;
    }

    if (!strcmp(words[0], "set")) {
        char key[50];
        char value[50];
        if (sscanf(line, "set %s %s", key, value) != 2) {
            printf("Key: %s\n", key);
            printf("Value: %s\n", value);
        }

        for (int ix = 0; ix < 1000; ix++) {
            if (env_vars[ix].key != NULL && !strcmp(env_vars[ix].key, key)) {
                env_vars[ix].value = strdup(value);
                break;
            }

            if (env_vars[ix].key == NULL) {
                env_vars[ix].key = strdup(key);
                env_vars[ix].value = strdup(value);
                break;
            }
        }

        return -1;
    }

    if (!strcmp(words[0], "unset")) {
        char key[50];

        if (sscanf(line, "unset %s", key) != 1) {
            printf("Key after scan is %s\n", key);
        }

        for (int ix = 0; ix < 1000; ix++) {
            if (env_vars[ix].key != NULL && !strcmp(env_vars[ix].key, key)) {
                env_vars[ix].value = "";

                break;
            }
        }
        return -1;
    }

    return 0;
}

int handle_pipe(char* words[], int size) {
    char pipe1_path[1000];
    char pipe2_path[1000];
    bool input_redir = false;
    bool output_redir = false;
    int last_element_index = 0;
    int commands_separator_ix = find_pipe(words);

    if (commands_separator_ix != -1) {
        char** first_cmd = (char**)malloc(sizeof(char*) * size + 1);
        char** second_cmd = (char**)malloc(sizeof(char*) * size + 1);

        for (int ix = 0; words[ix] != NULL; ix++) {
            last_element_index = ix;
        }

        copy_ptrs_from_to(first_cmd, words, 0, commands_separator_ix - 1);
        copy_ptrs_from_to(second_cmd, words, commands_separator_ix + 1,
                          last_element_index);

        if (!find_absolute_path(first_cmd[0], pipe1_path)) {
            printf("Command not found: %s\n", first_cmd[0]);
            return -1;
        }

        if (!find_absolute_path(second_cmd[0], pipe2_path)) {
            printf("Command not found: %s\n", second_cmd[0]);
            return -1;
        }

        int pipe_from_read_to_write[2];
        pipe(pipe_from_read_to_write);

        // find input redirection in first command
        for (int i = 0; first_cmd[i] != NULL; i++) {
            if (!strcmp(first_cmd[i], "<")) {
                input_redir = true;
                fd_input = open(first_cmd[i + 1], O_RDONLY);
                first_cmd[i] = NULL;
                first_cmd[i + 1] = NULL;
            }
        }

        // find output redirection in second command
        for (int i = 0; second_cmd[i] != NULL; i++) {
            if (!strcmp(second_cmd[i], ">")) {
                output_redir = true;
                fd_output =
                    open(second_cmd[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                second_cmd[i] = NULL;
                second_cmd[i + 1] = NULL;
            }
        }

        int command1_pid = fork();

        if (command1_pid == 0) {
            // first command process
            close(pipe_from_read_to_write[READ_SIDE]);
            dup2(pipe_from_read_to_write[WRITE_SIDE], STDOUT_FILENO);

            if (input_redir) {
                dup2(fd_input, STDIN_FILENO);
                close(fd_input);
            }

            execve(pipe1_path, first_cmd, NULL);

            fprintf(stderr, "Failed to execute first: %s\n", pipe1_path);
            // children should exit if exec fails
            _exit(1);
        }

        int command2_pid = fork();
        if (command2_pid == 0) {
            // second command process
            close(pipe_from_read_to_write[WRITE_SIDE]);
            dup2(pipe_from_read_to_write[READ_SIDE], STDIN_FILENO);

            if (output_redir) {
                dup2(fd_output, STDOUT_FILENO);
                close(fd_output);
            }
            execve(pipe2_path, second_cmd, NULL);

            fprintf(stderr, "Failed to execute second: %s\n", pipe2_path);
            // children should exit if exec fails
            _exit(1);
        }

        close(pipe_from_read_to_write[READ_SIDE]);
        close(pipe_from_read_to_write[WRITE_SIDE]);

        int wait_status;
        waitpid(command1_pid, &wait_status, 0);

        waitpid(command2_pid, &wait_status, 0);
    }

    return 0;
}

int handle_redir(char* words[], char* absolute_path) {
    fd_input = STDIN_FILENO;
    fd_output = STDOUT_FILENO;

    for (int ix = 0; words[ix] != NULL; ix++) {
        // check for redirection

        if (!strcmp(words[ix], "<")) {
            if (words[ix + 1] == NULL) {
                fprintf(stderr, "No input file specified\n");
                return -1;
            }

            fd_input = open(words[ix + 1], O_RDONLY);
            if (fd_input == -1) {
                fprintf(stderr, "Failed to open input file");
                return -1;
            }
            words[ix] = "";
            words[ix + 1] = "";
        }

        if (!strcmp(words[ix], ">")) {
            if (words[ix + 1] == NULL) {
                fprintf(stderr, "No output file specified\n");
                return -1;
            }

            fd_output = open(words[ix + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);

            words[ix] = NULL;

            if (fd_output == -1) {
                fprintf(stderr, "Failed to open output file");
                return -1;
            }

            break;
        }
    }

    // remove empty strings from the array
    for (int ix = 0; words[ix] != NULL; ix++) {
        if (words[ix] == "") {
            words[ix] = NULL;
        }
    }

    pid_t pid = fork();

    if (pid == 0) {
        if (fd_input != STDIN_FILENO) {
            dup2(fd_input, STDIN_FILENO);
            close(fd_input);
        }
        if (fd_output != STDOUT_FILENO) {
            dup2(fd_output, STDOUT_FILENO);
            close(fd_output);
        }

        execve(absolute_path, words, NULL);

        printf("Exec failed\n");
        _exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        if (fd_input != STDIN_FILENO) {
            close(fd_input);
        }
        if (fd_output != STDOUT_FILENO) {
            close(fd_output);
        }
    } else {
        perror("fork");
    }
}