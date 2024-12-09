#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>



void add_character_to_string(char *str, char c);

void split(char *cmd, char *words[], char delimiter);

void break_into_words(char *cmd, char *words[]);

bool find_absolute_path(char *cmd, char *absolute_path);

