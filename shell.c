#include "shell.h"

// Global history array and index
char *history[HISTORY_SIZE];
int history_count = 0;

int main(int argc, char **argv) {
    bool should_run = true;
    char *line = NULL;
    char *args[MAXLINE/2 + 1];
    
    while (should_run) {
        printf(PROMPT);
        fflush(stdout);
        
        // Read a line
        int n = fetchline(&line);
        if (n == -1) break;
        
        // Check for exit
        if (equal(line, "exit")) {
            should_run = false;
            continue;
        }
        
        // Handle special commands
        if (equal(line, "!!")) {
            executeHistoryCommand();
            continue;
        }
        if (equal(line, "ascii")) {
            ascii_art();
            addToHistory(line);
            continue;
        }
        
        // Tokenize the line
        int arg_count = tokenize(line, args);
        if (arg_count == 0) continue;
        
        // Process commands
        int start = 0;
        while (start < arg_count && args[start] != NULL) {
            CommandInfo cmd = {0};
            parse(args, start, &cmd);
            doCommand(args, &cmd);
            start = cmd.end + 1;
        }
        
        addToHistory(line);
    }
    
    // Cleanup
    if (line) free(line);
    for (int i = 0; i < history_count; i++) {
        free(history[i]);
    }
    return 0;
}

int tokenize(char *line, char *args[]) {
    char *token;
    char *saveptr;
    int count = 0;
    char *line_copy = strdup(line);
    
    // Initialize the first token
    token = strtok_r(line_copy, " \t", &saveptr);
    
    // Process all tokens
    while (token != NULL && count < MAXLINE/2) {
        // Check if token is a special character
        if (equal(token, "|")) {
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

void parse(char *args[], int start, CommandInfo *cmd) {
    cmd->start = start;
    cmd->waitFor = true;  // Default to waiting
    
    // Find end of command (NULL or ; or &)
    int i = start;
    while (args[i] != NULL) {
        if (equal(args[i], ";")) {
            args[i] = NULL;
            cmd->end = i - 1;
            break;
        }
        if (equal(args[i], "&")) {
            args[i] = NULL;
            cmd->end = i - 1;
            cmd->waitFor = false;
            break;
        }
        if (equal(args[i], "|")) {
            cmd->has_pipe = true;
            cmd->pipe_pos = i;
        }
        if (equal(args[i], "<")) {
            args[i] = NULL;
            cmd->input_file = args[i + 1];
            i++;
        }
        if (equal(args[i], ">")) {
            args[i] = NULL;
            cmd->output_file = args[i + 1];
            i++;
        }
        i++;
    }
    if (args[i] == NULL && !cmd->has_pipe) {
        cmd->end = i - 1;
    }
}

void doCommand(char *args[], CommandInfo *cmd) {
    // Handle pipe command
    if (cmd->has_pipe) {
        int pipefd[2];
        
        // Create pipe
        if (pipe(pipefd) == -1) {
            perror("pipe failed");
            return;
        }
        
        // Split commands at pipe
        args[cmd->pipe_pos] = NULL;
        char **cmd1 = &args[cmd->start];
        char **cmd2 = &args[cmd->pipe_pos + 1];
        
        // First process
        pid_t pid1 = fork();
        if (pid1 == 0) {
            dup2(pipefd[1], STDOUT_FILENO);  // Redirect stdout to pipe
            close(pipefd[0]);                // Close unused read end
            close(pipefd[1]);                // Close write end after dup2
            execvp(cmd1[0], cmd1);
            perror("First command failed");
            exit(1);
        }
        
        // Second process
        pid_t pid2 = fork();
        if (pid2 == 0) {
            dup2(pipefd[0], STDIN_FILENO);   // Redirect stdin from pipe
            close(pipefd[1]);                // Close unused write end
            close(pipefd[0]);                // Close read end after dup2
            execvp(cmd2[0], cmd2);
            perror("Second command failed");
            exit(1);
        }
        
        // Parent process
        close(pipefd[0]);
        close(pipefd[1]);
        waitpid(pid1, NULL, 0);
        waitpid(pid2, NULL, 0);
        
    } else {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork failed");
            return;
        }
        
        if (pid == 0) {
            // Handle redirections
            if (cmd->input_file) {
                int fd = open(cmd->input_file, O_RDONLY);
                if (fd < 0) {
                    perror("open input failed");
                    exit(1);
                }
                dup2(fd, STDIN_FILENO);
                close(fd);
            }
            
            if (cmd->output_file) {
                int fd = open(cmd->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0) {
                    perror("open output failed");
                    exit(1);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }
            
            execvp(args[cmd->start], &args[cmd->start]);
            perror("exec failed");
            exit(1);
        }
        
        if (cmd->waitFor) {
            waitpid(pid, NULL, 0);
        }
    }
}

// History and utility functions remain the same
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
    char *args[MAXLINE/2 + 1];
    int count = tokenize(history[history_count - 1], args);
    if (count > 0) {
        CommandInfo cmd = {0};
        parse(args, 0, &cmd);
        doCommand(args, &cmd);
        cleanup(args, count);
    }
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