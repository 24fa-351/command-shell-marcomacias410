#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// file used to parse the command line


void add_character_to_string(char *str, char c) {
    int len = strlen(str);
    str[len] = c;
    str[len + 1] = '\0';
}

void split(char *cmd, char *words[], char delimiter) {
    int word_count = 0;
    char *next_char = cmd;
    char current_word[1000];
    strcpy(current_word, "");

    while (*next_char != '\0') {
        if (*next_char == delimiter) {
            words[word_count++] = strdup(current_word);
            strcpy(current_word, "");

        } else {
            add_character_to_string(current_word, *next_char);
        }
        ++next_char;
    }

    words[word_count++] = strdup(current_word);
    words[word_count] = NULL;
}

void break_into_words(char *cmd, char *words[]) {
    words[0] = strdup(cmd);
    words[1] = NULL;
}

bool find_absolute_path(char *cmd, char *absolute_path) {
    char *directories[1000];

    split(getenv("PATH"), directories, ':');

    for (int ix = 0; directories[ix] != NULL; ix++) {
        char path[1000];
        strcpy(path, directories[ix]);
        add_character_to_string(path, '/');
        strcat(path, cmd);

        if (access(path, X_OK) == 0) {
            strcpy(absolute_path, path);
            return true;
        }
    }

    return false;
}

