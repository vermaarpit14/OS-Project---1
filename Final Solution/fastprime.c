#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>
#include <signal.h>

pid_t *child_pids = NULL;
int num_children = 0;

void handle_sigint(int sig) {
    if (child_pids != NULL) {
        for (int i = 0; i < num_children; i++) {
            if (child_pids[i] > 0) kill(child_pids[i], SIGKILL);
        }
    }
    printf("\nTerminated by user. Cleaning up.\n");
    exit(1);
}

int is_prime(int n) {
    if (n <= 1) return 0;
    if (n <= 3) return 1;
    if (n % 2 == 0 || n % 3 == 0) return 0;
    for (int i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) return 0;
    }
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <rl> <rh> <num_procs>\n", argv[0]);
        return 1;
    }

    int rl = atoi(argv[1]);
    int rh = atoi(argv[2]);
    int n_procs = atoi(argv[3]);

    if (n_procs < 1) n_procs = 1;

    signal(SIGINT, handle_sigint);

    child_pids = malloc(n_procs * sizeof(pid_t));
    num_children = n_procs;

    int fd_main = open("prime.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_main < 0) {
        perror("Error opening file");
        return 1;
    }
    close(fd_main);

    struct timeval start, end;
    gettimeofday(&start, NULL);

    int range = rh - rl + 1;
    int chunk_size = range / n_procs;

    for (int i = 0; i < n_procs; i++) {
        int chunk_start = rl + i * chunk_size;
        int chunk_end = (i == n_procs - 1) ? rh : (chunk_start + chunk_size - 1);

        pid_t pid = fork();

        if (pid < 0) {
            perror("Fork failed");
            exit(1);
        } else if (pid == 0) {
            int fd = open("prime.txt", O_WRONLY | O_CREAT | O_APPEND, 0644);
            char buffer[4096];
            int offset = 0;

            for (int num = chunk_start; num <= chunk_end; num++) {
                if (is_prime(num)) {
                    int len = sprintf(buffer + offset, "%d\n", num);
                    offset += len;

                    if (offset > 4000) {
                        write(fd, buffer, offset);
                        offset = 0;
                    }
                }
            }

            if (offset > 0) write(fd, buffer, offset);

            close(fd);
            exit(0);
        } else {
            child_pids[i] = pid;
        }
    }

    for (int i = 0; i < n_procs; i++) {
        wait(NULL);
    }

    gettimeofday(&end, NULL);
    double elapsed = (end.tv_sec - start.tv_sec) +
                     (end.tv_usec - start.tv_usec) / 1000000.0;

    printf("%f\n", elapsed);

    free(child_pids);
    return 0;
}
