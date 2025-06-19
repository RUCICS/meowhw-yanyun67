#include <stdio.h>
#include <unistd.h>  // 包含 read/write 等系统调用
#include <fcntl.h>   // 包含文件操作标志（如 O_RDONLY）
#include <stdlib.h>  // 包含 exit()

int main(int argc, char *argv[]) {
    // 检查参数数量，如果不是 2 个（程序名+文件名）就报错
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // 打开文件（O_RDONLY 表示只读模式）
    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {  // 如果打开失败
        perror("open");  // 打印错误信息（如 "open: No such file"）
        exit(EXIT_FAILURE);
    }

    // 核心逻辑：每次读 1 个字符，写到屏幕
    char c;  // 存储单个字符
    while (read(fd, &c, 1) > 0) {  // 从文件读 1 字节到 c
        if (write(STDOUT_FILENO, &c, 1) == -1) {  // 把 c 写到屏幕
            perror("write");
            close(fd);
            exit(EXIT_FAILURE);
        }
    }

    // 检查是否读取出错（read 返回 -1 表示错误）
    if (c == -1) {
        perror("read");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // 关闭文件
    close(fd);
    return EXIT_SUCCESS;  // 返回 0 表示成功
}