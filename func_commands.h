#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>



typedef struct env {
    char* key;
    char* value;
} env_vars_t;


int find_pipe(char* words[]);
void copy_ptrs_from_to(char** to, char** from, int from_ix, int to_ix);


int handle_redir(char* words[], char* absolute_path);
int handle_pipe(char* words[], int size);
int handle_internals(char* words[], char* line, env_vars_t env_vars[]);