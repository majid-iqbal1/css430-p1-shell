#include "shell.h"

// Global variables
char *args[MAXLINE/2 + 1];
char *history[HISTORY_SIZE];
int history_count = 0;

int main(int argc, char **argv) {
    if (argc == 2 && equal(argv[1], "--interactive")) {
        return interactiveShell();
    } else {
        return runTests();
    }
}

int interactiveShell() {
    bool should_run = true;
    char *line = NULL;
    
    while (should_run) {
        printf(PROMPT);
        fflush(stdout);
        int n = fetchline(&line);
        
        if (n == -1 || equal(line, "exit")) {
            should_run = false;
            continue;
        }
        if (equal(line, "")) {
            continue;
        }
        processLine(line);
    }
    
    // Cleanup history
    for (int i = 0; i < history_count; i++) {
        free(history[i]);
    }
    free(line);
    return 0;
}

void processLine(char *line) {
    if (equal(line, "!!")) {
        executeHistoryCommand();
        return;
    }
    
    if (equal(line, "ascii")) {
        ascii_art();
        addToHistory(line);
        return;
    }
    
    int arg_count = tokenize(line, args);
    if (arg_count == 0) return;

    int start = 0;
    while (start < arg_count && args[start] != NULL) {
        Command cmd = {0};
        parse(args, start, &cmd);
        doCommand(args, &cmd);
        
        // Move to next command
        start = cmd.end + 1;
        if (start < arg_count && args[start] != NULL && equal(args[start], ";")) {
            start++;
        }
    }

    addToHistory(line);
    cleanup(args, arg_count);
}

int tokenize(char *line, char *args[]) {
    char *token;
    char *saveptr;
    int count = 0;
    char *line_copy = strdup(line);
    
    token = strtok_r(line_copy, " \t", &saveptr);
    while (token != NULL && count < MAXLINE/2) {
        if (equal(token, "|") || equal(token, "&") || equal(token, ";")) {
            args[count++] = strdup(token);
        } else {
            args[count++] = strdup(token);
        }
        token = strtok_r(NULL, " \t", &saveptr);
    }
    
    args[count] = NULL;
    free(line_copy);
    return count;
}

void parse(char *args[], int start, Command *cmd) {
    cmd->start = start;
    cmd->waitFor = true;
    cmd->has_pipe = false;
    cmd->pipe_pos = -1;
    cmd->input_file = NULL;
    cmd->output_file = NULL;
    
    int i = start;
    while (args[i] != NULL) {
        if (equal(args[i], "|")) {
            cmd->has_pipe = true;
            cmd->pipe_pos = i;
            args[i] = NULL;
            cmd->end = i;
            return;
        }
        
        if (equal(args[i], "&")) {
            cmd->background = true;
            args[i] = NULL;
            cmd->end = i - 1;
            cmd->waitFor = false;
            return;
        }
        
        if (equal(args[i], ";")) {
            args[i] = NULL;
            cmd->end = i - 1;
            cmd->waitFor = true;
            return;
        }
        
        if (equal(args[i], "<")) {
            args[i] = NULL;
            cmd->input_file = args[i + 1];
            i++;
            continue;
        }
        
        if (equal(args[i], ">")) {
            args[i] = NULL;
            cmd->output_file = args[i + 1];
            i++;
            continue;
        }
        
        i++;
    }
    cmd->end = i - 1;
}

void doCommand(char *args[], Command *cmd) {
    if (cmd->has_pipe) {
        int pipefd[2];
        
        if (pipe(pipefd) == -1) {
            perror("pipe failed");
            return;
        }
        
        // First process
        pid_t pid1 = fork();
        if (pid1 == 0) {
            dup2(pipefd[1], STDOUT_FILENO);
            close(pipefd[0]);
            close(pipefd[1]);
            
            execvp(args[cmd->start], &args[cmd->start]);
            perror("First command failed");
            exit(1);
        }
        
        // Second process
        pid_t pid2 = fork();
        if (pid2 == 0) {
            dup2(pipefd[0], STDIN_FILENO);
            close(pipefd[0]);
            close(pipefd[1]);
            
            execvp(args[cmd->pipe_pos + 1], &args[cmd->pipe_pos + 1]);
            perror("Second command failed");
            exit(1);
        }
        
        close(pipefd[0]);
        close(pipefd[1]);
        
        waitpid(pid1, NULL, 0);
        waitpid(pid2, NULL, 0);
    } else {
        pid_t pid = fork();
        
        if (pid == 0) {
            if (cmd->input_file || cmd->output_file) {
                handleRedirection(cmd);
            }
            
            execvp(args[cmd->start], &args[cmd->start]);
            perror("Command execution failed");
            exit(1);
        } else {
            if (cmd->waitFor) {
                waitpid(pid, NULL, 0);
            }
        }
    }
}

void handleRedirection(Command *cmd) {
    int fd;
    
    if (cmd->input_file) {
        fd = open(cmd->input_file, O_RDONLY);
        if (fd < 0) {
            perror("Input file open failed");
            exit(1);
        }
        dup2(fd, STDIN_FILENO);
        close(fd);
    }
    
    if (cmd->output_file) {
        fd = open(cmd->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) {
            perror("Output file open failed");
            exit(1);
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
}

void addToHistory(char *command) {
    if (history_count < HISTORY_SIZE) {
        history[history_count] = strdup(command);
        history_count++;
    } else {
        free(history[0]);
        for (int i = 0; i < HISTORY_SIZE - 1; i++) {
            history[i] = history[i + 1];
        }
        history[HISTORY_SIZE - 1] = strdup(command);
    }
}

void executeHistoryCommand() {
    if (history_count == 0) {
        printf("No commands in history.\n");
        return;
    }
    
    printf("%s\n", history[history_count - 1]);
    processLine(history[history_count - 1]);
}

void ascii_art() {
    printf("  |\\_/|        ****************************    (\\_/)\n");
    printf(" / @ @ \\       *  \"Purrrfectly pleasant\"  *   (='.'^)\n");
    printf("( > º < )      *       Poppy Prinz        *   (\")_(\")\n");
    printf(" `>>x<<´       *   (pprinz@example.com)   *\n");
    printf(" /  O  \\       ****************************\n");
}

void cleanup(char *args[], int count) {
    for (int i = 0; i < count; i++) {
        if (args[i]) free(args[i]);
        args[i] = NULL;
    }
}

bool equal(char *a, char *b) {
    if (!a || !b) return false;
    return strcmp(a, b) == 0;
}

int fetchline(char **line) {
    size_t len = 0;
    ssize_t n = getline(line, &len, stdin);
    if (n > 0) {
        (*line)[n - 1] = '\0';  // Remove newline
    }
    return n;
}

int runTests() {
    printf("*** Running basic tests ***\n");
    char *tests[] = {
        "ls",
        "ls -al",
        "ls & whoami ;",
        "ls > junk.txt",
        "cat < junk.txt",
        "ls | wc",
        "ascii"
    };
    
    for (int i = 0; i < 7; i++) {
        printf("* Testing: %s\n", tests[i]);
        processLine(tests[i]);
    }
    
    return 0;
}