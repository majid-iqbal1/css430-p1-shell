# UNIX Shell Implementation

## Overview
A simple UNIX shell implementation that supports basic command execution, I/O redirection, pipes, and command history.

## Features
* Basic command execution (e.g., `ls`, `pwd`)
* Background processes with `&` operator
* Command sequencing with `;` operator
* Input/Output redirection (`<` and `>`)
* Pipe support (`|`)
* Command history (`!!`)
* ASCII art easter egg

## Building
```bash
# Compile the project
gcc -Wall -Wextra *.c
```

## Running
```bash
# Run in interactive mode
./a.out --interactive

# Run tests
./a.out
```

## Example Commands
```bash
# Basic commands
osh> ls
osh> pwd

# Background process with semicolon
osh> ls & whoami ;

# I/O redirection
osh> ls > output.txt
osh> cat < input.txt

# Pipe
osh> ls | wc

# Command history
osh> ls -l
osh> !!    # repeats last command

# Easter egg
osh> ascii
```

## Exit
Type `exit` or press `Ctrl-D` to exit the shell.