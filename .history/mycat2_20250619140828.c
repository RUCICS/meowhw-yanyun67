#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>

// 获取系统内存页大小（动态检测）
size_t io_blocksize() {
    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size == -1) {
        perror("sysconf failed, using fallback 4096");
        return 4096; // 失败时回退到常见值
    }
    return (size_t)page_size;
}

int main(int argc, char *argv[]) {
    // 参数检查
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // 打开文件
    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // 动态分配缓冲区
    size_t buf_size = io_blocksize();
    char *buffer = malloc(buf_size);
    if (!buffer) {
        perror("malloc");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // 带缓冲的读写
    ssize_t bytes_read;
    while ((bytes_read = read(fd, buffer, buf_size)) > 0) {
        if (write(STDOUT_FILENO, buffer, bytes_read) == -1) {
            perror("write");
            free(buffer);
            close(fd);
            exit(EXIT_FAILURE);
        }
    }

    // 错误处理
    if (bytes_read == -1) {
        perror("read");
        free(buffer);
        close(fd);
        exit(EXIT_FAILURE);
    }

    // 清理资源
    free(buffer);
    close(fd);
    return EXIT_SUCCESS;
}