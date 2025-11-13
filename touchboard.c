/*************************************************************************
    > File Name: touchboard.c
    > Author: bxd
    > Mail: 690489149@qq.com 
    > Created Time: Wed Apr 23 01:43:12 2025
 ************************************************************************/
#if 0
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

#endif
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <linux/input.h>

#define TOUCH_DEV "/dev/input/event2"  // 修改为实际设备节点

// 全局坐标存储结构体（带互斥锁）
typedef struct {
    int x;
    int y;
    pthread_mutex_t lock;
    int fd;             // 设备文件描述符
    int running;        // 线程运行标志
    void (*callback)(int x, int y);  // 坐标更新回调函数
} TouchState;

// 初始化全局状态
static TouchState touch_state = { 
    .x = 0, 
    .y = 0,
    .lock = PTHREAD_MUTEX_INITIALIZER,
    .fd = -1,
    .running = 0,
    .callback = NULL
};

// 设置回调函数
void set_touch_callback(void (*callback)(int x, int y)) {
    pthread_mutex_lock(&touch_state.lock);
    touch_state.callback = callback;
    pthread_mutex_unlock(&touch_state.lock);
}

// 线程函数：持续读取触摸事件
void* touch_thread(void* arg) {
    struct input_event ev;
    int rel_x = 0, rel_y = 0;
    int btn_touch = 0;  // 触摸状态

    while (touch_state.running) {
        if (read(touch_state.fd, &ev, sizeof(ev)) < (ssize_t)sizeof(ev)) {
            perror("读取事件失败");
            continue;
        }

        // 处理相对移动事件
        if (ev.type == EV_REL) {
            pthread_mutex_lock(&touch_state.lock);
            switch (ev.code) {
                case REL_X: 
                    rel_x += ev.value;
                    //touch_state.x += ev.value;  // 累加相对位移
                    touch_state.x = ev.value;  
                    break;
                case REL_Y: 
                    rel_y += ev.value;
                    //touch_state.y += ev.value;
                    touch_state.y = ev.value;
                    break;
            }
            pthread_mutex_unlock(&touch_state.lock);
        }

        // 处理触摸状态（如果设备支持）
        if (ev.type == EV_KEY && ev.code == BTN_TOUCH) {
            btn_touch = ev.value;
            if (!btn_touch) {  // 手指抬起时重置相对量
                rel_x = rel_y = 0;
                printf("touch end!\n");
            }
        }

        // 同步事件处理
        if (ev.type == EV_SYN) {
            printf("ev_syn\n");
            if (touch_state.callback) {
                pthread_mutex_lock(&touch_state.lock);
                touch_state.callback(touch_state.x, touch_state.y);
                pthread_mutex_unlock(&touch_state.lock);
            }
            printf("当前坐标: X=%d, Y=%d\n", touch_state.x, touch_state.y);
        }
        usleep(25000);
    }

    return NULL;
}

// 初始化触摸设备
int touch_init() {
    touch_state.fd = open(TOUCH_DEV, O_RDONLY);
    if (touch_state.fd < 0) {
        perror("无法打开触摸设备");
        return -1;
    }

    touch_state.running = 1;
    pthread_t tid;
    if (pthread_create(&tid, NULL, touch_thread, NULL) != 0) {
        close(touch_state.fd);
        return -1;
    }
    return 0;
}

// 获取当前坐标（线程安全）
void get_current_coordinates(int *x, int *y) {
    pthread_mutex_lock(&touch_state.lock);
    *x = touch_state.x;
    *y = touch_state.y;
    pthread_mutex_unlock(&touch_state.lock);
}

// 关闭设备
void touch_exit() {
    touch_state.running = 0;
    close(touch_state.fd);
    pthread_mutex_destroy(&touch_state.lock);
}

// 示例使用
int main() {
    printf("等待100ms后初始化触摸设备...\n");
    usleep(100000);  // 100ms = 100000微秒
    
    if (touch_init() != 0) {
        fprintf(stderr, "初始化失败\n");
        return 1;
    }

    // 主循环中可随时获取坐标
    while (1) {
        // int curr_x, curr_y;
        // get_current_coordinates(&curr_x, &curr_y);
        // printf("当前坐标: X=%d, Y=%d\n", curr_x, curr_y);
        // usleep(1000000);  // 延时以便观察
    }

    touch_exit();
    return 0;
}