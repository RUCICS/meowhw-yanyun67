#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>

// 获取系统内存页大小
size_t io_blocksize() {
    static long page_size = -1;
    if (page_size == -1) {
        page_size = sysconf(_SC_PAGESIZE);
        if (page_size == -1) {
            perror("sysconf failed, using fallback 4096");
            page_size = 4096;
        }
    }
    return (size_t)page_size;
}

// 分配对齐到内存页的内存
void* align_alloc(size_t size) {
    size_t page_size = io_blocksize();
    size_t request_size = size + page_size - 1;
    
    void* raw_ptr = malloc(request_size + sizeof(void*));
    if (!raw_ptr) return NULL;

    // 计算对齐地址
    void* aligned_ptr = (void*)(((size_t)raw_ptr + sizeof(void*) + page_size - 1) & ~(page_size - 1));
    
    // 在对齐地址前存储原始指针
    ((void**)aligned_ptr)[-1] = raw_ptr;
    
    return aligned_ptr;
}

// 释放对齐内存
void align_free(void* ptr) {
    if (ptr) {
        free(((void**)ptr)[-1]);
    }
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

    size_t buf_size = io_blocksize();
    char* buffer = align_alloc(buf_size);
    if (!buffer) {
        perror("align_alloc");
        close(fd);
        return EXIT_FAILURE;
    }

    ssize_t bytes_read;
    while ((bytes_read = read(fd, buffer, buf_size)) > 0) {
        if (write(STDOUT_FILENO, buffer, bytes_read) == -1) {
            perror("write");
            align_free(buffer);
            close(fd);
            return EXIT_FAILURE;
        }
    }

    if (bytes_read == -1) {
        perror("read");
    }

    align_free(buffer);
    close(fd);
    return (bytes_read == -1) ? EXIT_FAILURE : EXIT_SUCCESS;
}