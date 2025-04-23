/*************************************************************************
    > File Name: touchboard.c
    > Author: bxd
    > Mail: 690489149@qq.com 
    > Created Time: Wed Apr 23 01:43:12 2025
 ************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>

#define TOUCH_DEVICE "/dev/input/event2"  // 修改为你的设备节点

int main() {
    int fd;
    struct input_event ev;
    int x = 0, y = 0;
    
    // 打开输入设备
    fd = open(TOUCH_DEVICE, O_RDONLY);
    if (fd == -1) {
        perror("无法打开设备");
        exit(EXIT_FAILURE);
    }

    printf("正在监听触摸事件... (Ctrl+C退出)\n");

    // 持续读取输入事件
    while (1) {
        ssize_t bytes = read(fd, &ev, sizeof(ev));
        if (bytes < 0) {
            perror("读取事件失败");
            break;
        }

        // 解析坐标事件
        if (ev.type == EV_ABS) {
            switch (ev.code) {
                case ABS_X:  // X轴坐标
                    x = ev.value;
                    break;
                case ABS_Y:  // Y轴坐标
                    y = ev.value;
                    break;
            }
        }
        else if (ev.type == EV_REL)
        {
            /* code */
            switch (ev.code)
            {
                case REL_X:
                    x = ev.value;
                    break;
                case REL_Y:
                    y = ev.value;
                    break;
            }
           
        }
        

        // 当有坐标更新时输出
        if (ev.type == EV_SYN) {  // 同步事件表示一组数据结束
            printf("坐标: X=%d, Y=%d\n", x, y);
        }
    }

    close(fd);
    return 0;
}

