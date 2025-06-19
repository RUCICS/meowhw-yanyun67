#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>

// 获取系统内存页大小
static size_t get_page_size() {
    static long page_size = -1;
    if (page_size == -1) {
        page_size = sysconf(_SC_PAGESIZE);
        if (page_size == -1) {
            perror("sysconf(_SC_PAGESIZE) failed");
            page_size = 4096; // 默认值
        }
    }
    return (size_t)page_size;
}

// 获取文件系统块大小
static size_t get_filesystem_blocksize(const char* filename) {
    struct stat st;
    if (stat(filename, &st) == -1) {
        perror("stat failed");
        return 4096; // 默认值
    }

    // 处理虚假块大小（确保是2的幂）
    size_t block_size = (size_t)st.st_blksize;
    if ((block_size & (block_size - 1)) != 0) { // 不是2的幂
        block_size = 4096; // 回退到常见值
    }
    return block_size;
}

// 综合考虑内存页和文件系统块大小
size_t io_blocksize(const char* filename) {
    size_t page_size = get_page_size();
    size_t fs_block_size = get_filesystem_blocksize(filename);

    // 取两者的最小公倍数
    size_t lcm = page_size > fs_block_size ? page_size : fs_block_size;
    while (lcm % page_size != 0 || lcm % fs_block_size != 0) {
        lcm += page_size;
    }

    return lcm;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // 动态计算缓冲区大小
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

    if (bytes_read == -1) {
        perror("read");
    }

    free(buffer);
    close(fd);
    return (bytes_read == -1) ? EXIT_FAILURE : EXIT_SUCCESS;
}