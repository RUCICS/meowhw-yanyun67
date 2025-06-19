#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

#define OPTIMAL_MULTIPLIER 16  // 根据任务5实验结果设置

size_t io_blocksize(const char* filename) {
    struct stat st;
    if (stat(filename, &st) == -1) {
        perror("stat failed");
        return 4096 * OPTIMAL_MULTIPLIER;
    }
    return st.st_blksize * OPTIMAL_MULTIPLIER;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("open");
        return EXIT_FAILURE;
    }

    // 关键优化：提示文件将顺序读取
    if (posix_fadvise(fd, 0, 0, POSIX_FADV_SEQUENTIAL) == -1) {
        perror("fadvise failed");  // 非致命错误，继续运行
    }

    size_t buf_size = io_blocksize(argv[1]);
    char* buffer = malloc(buf_size);
    if (!buffer) {
        perror("malloc");
        close(fd);
        return EXIT_FAILURE;
    }

    ssize_t bytes_read;
    while ((bytes_read = read(fd, buffer, buf_size)) > 0) {
        if (write(STDOUT_FILENO, buffer, bytes_read) == -1) {
            perror("write");
            break;
        }
    }

    if (bytes_read == -1) perror("read");

    free(buffer);
    close(fd);
    return (bytes_read == -1) ? EXIT_FAILURE : EXIT_SUCCESS;
}