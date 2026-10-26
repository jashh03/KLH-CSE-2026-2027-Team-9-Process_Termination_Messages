#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    if (argc != 2 || argv[1][0] == '\0') {
        fprintf(stderr, "Usage: %s \"command\"\n", argv[0]);
        return 2;
    }

    printf("[C ENGINE] Starting Linux job execution...\n");
    fflush(stdout);

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        execl("/bin/sh", "sh", "-c", argv[1], (char *)NULL);
        perror("exec");
        _exit(127);
    }

    int status;
    while (waitpid(pid, &status, 0) < 0) {
        if (errno == EINTR) continue;
        perror("waitpid");
        return 1;
    }

    if (WIFEXITED(status)) {
        int code = WEXITSTATUS(status);
        printf("[C ENGINE] Process finished with exit code %d\n", code);
        return code;
    }

    fprintf(stderr, "[C ENGINE] Process terminated abnormally\n");
    return 1;
}
