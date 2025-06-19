#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        exit(1);
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(1);
    }

    char c = 0;
    ssize_t bytes_read;
    while ((bytes_read = read(fd, &c, 1)) > 0) {
        if (write(STDOUT_FILENO, &c, 1) == -1) {
            perror("write");
            close(fd);
            exit(1);
        }
    }

    if (bytes_read == -1) {
        perror("read");
        close(fd);
        exit(1);
    }

    close(fd);
    return 0;
}