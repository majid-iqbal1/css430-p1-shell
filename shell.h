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

// Global args array for commands
extern char *args[MAXLINE/2 + 1];

// Command structure for storing command information
typedef struct {
    int start;              // Start index in args array
    int end;               // End index in args array
    bool background;       // Run in background?
    bool has_pipe;         // Contains pipe?
    bool waitFor;          // Wait for completion?
    char *input_file;      // Input redirection file
    char *output_file;     // Output redirection file
    int pipe_pos;          // Position of pipe in args
} Command;

// Core functions
int main(int argc, char **argv);
bool equal(char *a, char *b);
int fetchline(char **line);
int interactiveShell();
int runTests();

// Command processing
void processLine(char *line);
int tokenize(char *line, char *args[]);
void parse(char *args[], int start, Command *cmd);
void doCommand(char *args[], Command *cmd);
void executeCommand(Command *cmd);
void handleRedirection(Command *cmd);
void cleanup(char *args[], int count);

// History feature
void addToHistory(char *command);
void executeHistoryCommand();

// Easter egg
void ascii_art();

#endif