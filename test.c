#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

void configure_terminal(int enable) {
    struct termios t;
    tcgetattr(STDIN_FILENO, &t); // 获取当前终端设置

    if (!enable) {
        // 禁用回显并启用非规范模式
        t.c_lflag &= ~(ECHO | ICANON);
    } else {
        // 恢复回显和规范模式
        t.c_lflag |= (ECHO | ICANON);
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &t); // 应用新的终端设置
}

int main() {
    // 禁用回显和行缓冲（进入非规范模式）
    configure_terminal(0);

    // 向终端发送请求光标位置的 ANSI 转义序列
    printf("\033[6n");
    fflush(stdout);

    // 接收终端返回的光标位置
    char buf[32];
    int i = 0;
    while (1) {
        if (read(STDIN_FILENO, &buf[i], 1) == -1) {
            perror("read");
            configure_terminal(1); // 恢复终端设置
            return 1;
        }
        if (buf[i] == 'R') break; // 如果遇到 'R'，表示数据结束
        i++;
    }

    // 恢复终端设置
    configure_terminal(1);

    // 确保字符串正确终止
    buf[i] = '\0';

    // 跳过前面的转义符 `\033[` (即 ^[)，从第 2 个字符开始解析
    int row, col;
    if (sscanf(buf + 2, "%d;%d", &row, &col) == 2) {
        // 打印光标位置
        printf("光标位置: 行 %d, 列 %d\n", row, col);
    } else {
        printf("无法解析光标位置\n");
    }

    return 0;
}
