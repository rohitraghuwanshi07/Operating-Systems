#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

#define MAX_ARGS 64
#define MAX_BG 64

pid_t bg_pids[MAX_BG];
int bg_count = 0;

void add_bg(pid_t pid) {
    if (bg_count < MAX_BG) {
        bg_pids[bg_count++] = pid;
    }
}

void remove_bg(pid_t pid) {
    for (int i = 0; i < bg_count; i++) {
        if (bg_pids[i] == pid) {
            bg_pids[i] = bg_pids[bg_count - 1];
            bg_count--;
            return;
        }
    }
}

int is_bg_pid(pid_t pid) {
    for (int i = 0; i < bg_count; i++)
        if (bg_pids[i] == pid) return 1;
    return 0;
}

// Non-blocking reap of any finished background children
void reap_background() {
    pid_t pid;
    int status;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (is_bg_pid(pid)) {
            remove_bg(pid);
            printf("Shell: Background process finished\n");
        }
    }
}

int main() {
    char line[1024];

    while (1) {
        reap_background();          // check for finished bg processes before prompting
        printf("myshell> ");
        fflush(stdout);

        if (fgets(line, sizeof(line), stdin) == NULL) break;

        // strip newline
        line[strcspn(line, "\n")] = '\0';
        if (strlen(line) == 0) continue;

        int background = 0;
        // check for trailing " &"
        int len = strlen(line);
        if (len >= 2 && line[len - 1] == '&' && line[len - 2] == ' ') {
            background = 1;
            line[len - 2] = '\0';   // strip " &"
        } else if (len >= 1 && line[len - 1] == '&') {
            background = 1;
            line[len - 1] = '\0';
        }

        // tokenize into argv
        char *args[MAX_ARGS];
        int argc = 0;
        char *token = strtok(line, " ");
        while (token != NULL && argc < MAX_ARGS - 1) {
            args[argc++] = token;
            token = strtok(NULL, " ");
        }
        args[argc] = NULL;

        if (argc == 0) continue;

        pid_t pid = fork();

        if (pid < 0) {
            perror("fork failed");
            continue;
        } else if (pid == 0) {
            // Child: exec the command
            execvp(args[0], args);
            perror("exec failed");
            exit(1);
        } else {
            // Parent
            if (background) {
                add_bg(pid);
                printf("Shell: Started background process %d\n", pid);
                // do NOT wait — return to prompt immediately
            } else {
                int status;
                pid_t wpid;
                do {
                    wpid = waitpid(pid, &status, 0);
                } while (wpid == -1 && errno == EINTR);
            }
        }
    }

    return 0;
}
