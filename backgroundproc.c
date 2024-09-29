#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

void sigchld_handler(int sig) {
    // Wait for any child process (non-blocking)
    while (waitpid(-1, NULL, WNOHANG) > 0) {
        // Reap all terminated children
        printf("SIGCHLD received: Child process has ended.\n");
    }
}

int main() {
    // Set up the SIGCHLD handler
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Child process
        char *args[] = {"testproc", NULL}; // Command to run
        char *envp[] = {NULL}; // Environment variables
        if (execve("testproc", args, envp) == -1) {
            perror("execve");
            exit(EXIT_FAILURE);
        }
    } else {
        // Parent process continues
        printf("Child process started with PID: %d\n", pid);

        // Parent can continue doing other work
        while (1) {
            sleep(1); // Simulate work being done
        }
    }

    return 0;
}

