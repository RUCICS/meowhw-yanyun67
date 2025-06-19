#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

// 实验得出的最优倍数（根据脚本输出调整）
#define OPTIMAL_MULTIPLIER 16

size_t io_blocksize(const char* filename) {
    static size_t optimal_size = 0;
    if (optimal_size == 0) {
        // 获取基础块大小
        struct stat st;
        if (stat(filename, &st) == -1) {
            perror("stat failed");
            return 4096 * OPTIMAL_MULTIPLIER;
        }

        size_t base_size = st.st_blksize;
        long page_size = sysconf(_SC_PAGESIZE);
        if (page_size == -1) page_size = 4096;

        // 计算基础对齐大小
        base_size = (base_size > (size_t)page_size) ? base_size : (size_t)page_size;
        optimal_size = base_size * OPTIMAL_MULTIPLIER;
    }
    return optimal_size;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    size_t buf_size = io_blocksize(argv[1]);
    char* buffer = malloc(buf_size);
    if (!buffer) {
        perror("malloc");
        return EXIT_FAILURE;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("open");
        free(buffer);
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