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
#include <pthread.h>
#include <linux/input.h>
#include <time.h>

#define TOUCH_DEV "/dev/input/event2"  // 修改为实际设备节点
#define CALLBACK_INTERVAL_MS 10        // 回调处理间隔（毫秒）

// 全局坐标存储结构体（带互斥锁和条件变量）
typedef struct {
    int x;
    int y;
    int has_new_data;           // 是否有新数据标志
    pthread_mutex_t lock;
    pthread_cond_t data_ready;   // 数据就绪条件变量
    int fd;                     // 设备文件描述符
    int running;                // 线程运行标志
    void (*callback)(int x, int y);  // 坐标更新回调函数
} TouchState;

// 初始化全局状态
static TouchState touch_state = {
    .x = 0, 
    .y = 0,
    .has_new_data = 0,
    .lock = PTHREAD_MUTEX_INITIALIZER,
    .data_ready = PTHREAD_COND_INITIALIZER,
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

// 线程1：持续读取触摸事件
void* touch_read_thread(void* arg) {
    struct input_event ev;
    
    printf("触摸读取线程启动\n");
    
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
                    touch_state.x = ev.value;
                    touch_state.has_new_data = 1;
                    break;
                case REL_Y: 
                    touch_state.y = ev.value;
                    touch_state.has_new_data = 1;
                    break;
            }
            
            // 如果有新数据，通知回调处理线程
            if (touch_state.has_new_data) {
                pthread_cond_signal(&touch_state.data_ready);
            }
            pthread_mutex_unlock(&touch_state.lock);
        }

        // 处理触摸状态（如果设备支持）
        if (ev.type == EV_KEY && ev.code == BTN_TOUCH) {
            if (!ev.value) {  // 手指抬起
                printf("touch end!\n");
            }
        }

        // 同步事件处理
        if (ev.type == EV_SYN) {
            printf("ev_syn - 当前坐标: X=%d, Y=%d\n", touch_state.x, touch_state.y);
        }
    }
    
    printf("触摸读取线程退出\n");
    return NULL;
}

// 线程2：定时处理回调
void* touch_callback_thread(void* arg) {
    struct timespec ts;
    int timeout_ms = CALLBACK_INTERVAL_MS;
    
    printf("回调处理线程启动，处理间隔: %dms\n", timeout_ms);
    
    while (touch_state.running) {
        pthread_mutex_lock(&touch_state.lock);
        
        // 等待数据就绪或超时
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_nsec += timeout_ms * 1000000L;
        if (ts.tv_nsec >= 1000000000L) {
            ts.tv_sec++;
            ts.tv_nsec -= 1000000000L;
        }
        
        // 等待条件变量，最多等待指定时间
        pthread_cond_timedwait(&touch_state.data_ready, &touch_state.lock, &ts);
        
        // 检查是否有新数据需要处理
        if (touch_state.has_new_data && touch_state.callback) {
            // 调用回调函数
            touch_state.callback(touch_state.x, touch_state.y);
            touch_state.has_new_data = 0;  // 重置标志
        }
        
        pthread_mutex_unlock(&touch_state.lock);
    }
    
    printf("回调处理线程退出\n");
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
    
    // 创建两个线程
    pthread_t read_tid, callback_tid;
    
    if (pthread_create(&read_tid, NULL, touch_read_thread, NULL) != 0) {
        close(touch_state.fd);
        return -1;
    }
    
    if (pthread_create(&callback_tid, NULL, touch_callback_thread, NULL) != 0) {
        touch_state.running = 0;
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
    
    // 通知所有等待的线程
    pthread_mutex_lock(&touch_state.lock);
    pthread_cond_broadcast(&touch_state.data_ready);
    pthread_mutex_unlock(&touch_state.lock);
    
    // 关闭文件描述符
    close(touch_state.fd);
    
    // 销毁同步对象
    pthread_mutex_destroy(&touch_state.lock);
    pthread_cond_destroy(&touch_state.data_ready);
    
    printf("触摸设备已关闭\n");
}

// 示例回调函数
void example_callback(int x, int y) {
    printf("回调处理: X=%d, Y=%d\n", x, y);
}

// 示例使用
int main() {
    printf("等待100ms后初始化触摸设备...\n");
    usleep(100000);  // 100ms = 100000微秒
    
    // 设置回调函数
    set_touch_callback(example_callback);
    
    if (touch_init() != 0) {
        fprintf(stderr, "初始化失败\n");
        return 1;
    }

    printf("触摸设备初始化成功，按Ctrl+C退出\n");
    
    // 主循环保持运行
    while (1) {
        sleep(1);
    }

    touch_exit();
    return 0;
}