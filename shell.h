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

#define RD 0
#define WR 1

typedef struct {
    char *args[MAXLINE/2 + 1];  
    int arg_count;              
    bool background;            
    bool pipe_exists;           
    char *input_file;          
    char *output_file;        
} Command;

bool equal(char *a, char *b);
int fetchline(char **line);
int interactiveShell();
int runTests();
void processLine(char *line);
void parseCommand(char *line, Command *cmd);
void executeCommand(Command *cmd);
void addToHistory(char *command);
void executeHistoryCommand();
void executePipedCommand(Command *cmd1, Command *cmd2);
void handleRedirection(Command *cmd);
void ascii_art();
void cleanup_command(Command *cmd);

#endif