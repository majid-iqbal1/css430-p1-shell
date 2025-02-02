#ifndef SHELL_H_
#define SHELL_H_

#include <assert.h>  // assert
#include <fcntl.h>   // O_RDWR, O_CREAT
#include <stdbool.h> // bool
#include <stdio.h>   // printf, getline
#include <stdlib.h>  // calloc
#include <string.h>  // strcmp
#include <unistd.h>  // execvp
#include <sys/wait.h> // wait
#include <ctype.h>   // isspace

#define MAXLINE 80
#define PROMPT "osh> "
#define HISTORY_SIZE 10

extern char *args[MAXLINE/2 + 1];

typedef struct {
    int start;
    int end;
    bool background;
    bool has_pipe;
    bool waitFor;
    char *input_file;
    char *output_file;
    int pipe_pos;
    int pipe_start;
    int pipe_end;
} Command;

bool equal(char *a, char *b);
int fetchline(char **line);
int interactiveShell();
int runTests();
void processLine(char *line);
int main(int argc, char **argv);
int tokenize(char *line, char *args[]);
int parse(char *args[], int start, Command *cmd);
void doCommand(char *args[], Command *cmd);
void executeCommand(Command *cmd);
void handleRedirection(Command *cmd);
void cleanup(char *args[], int count);

void addToHistory(char *command);
void executeHistoryCommand();

void ascii_art();

#endif