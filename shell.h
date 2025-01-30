#ifndef SHELL_H
#define SHELL_H

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

typedef struct {
    int start;          // Start index in args array
    int end;           // End index in args array
    bool waitFor;      // Should parent wait for this command?
    char *input_file;  // Input redirection file
    char *output_file; // Output redirection file
    bool has_pipe;     // Does this command have a pipe?
    int pipe_pos;      // Position of pipe in args
} CommandInfo;

// Function declarations
bool equal(char *a, char *b);
int fetchline(char **line);
int tokenize(char *line, char *args[]);
void parse(char *args[], int start, CommandInfo *cmd);
void doCommand(char *args[], CommandInfo *cmd);
void addToHistory(char *command);
void executeHistoryCommand();
void ascii_art();
void cleanup(char *args[], int count);

#endif