# 触摸板监控程序 (Touchboard Monitor)

一个基于Linux输入子系统的触摸板监控程序，采用双线程架构实现高效的实时触摸事件处理。

## 项目概述

本项目是一个C语言编写的触摸板监控工具，能够实时捕获和处理触摸板输入事件，并通过回调机制以固定频率处理坐标数据。

## 特性

- **双线程架构**：分离数据读取和数据处理，提高系统响应性
- **实时事件处理**：毫秒级精度的触摸事件捕获
- **线程安全**：使用互斥锁和条件变量确保数据一致性
- **可配置回调**：支持自定义回调函数处理触摸数据
- **优雅的资源管理**：完整的初始化-使用-清理生命周期

## 系统要求

- Linux操作系统
- 支持输入子系统的触摸板设备
- GCC编译器
- pthread库

## 编译和运行

### 编译项目

```bash
make
```

### 运行程序

```bash
./touch_monitor
```

### 清理编译文件

```bash
make clean
```

## 项目结构

```
touchboardtest/
├── touchboard.c      # 主程序源代码
├── makefile          # 编译配置文件
├── README.md         # 项目说明文档
└── obj/             # 编译输出目录
```

## 核心功能

### 双线程架构

1. **触摸读取线程**
   - 持续从设备文件读取输入事件
   - 处理相对坐标事件(EV_REL)
   - 实时更新坐标数据
   - 通过条件变量通知处理线程

2. **回调处理线程**
   - 以固定频率(默认10ms)处理回调
   - 使用条件变量定时唤醒机制
   - 调用用户定义的回调函数

### API接口

```c
// 设置触摸事件回调函数
void set_touch_callback(void (*callback)(int x, int y));

// 初始化触摸设备
int touch_init();

// 获取当前坐标（线程安全）
void get_current_coordinates(int *x, int *y);

// 关闭设备并清理资源
void touch_exit();
```

## 配置说明

### 设备路径配置

在`touchboard.c`中修改`TOUCH_DEV`宏定义，指向正确的输入设备：

```c
#define TOUCH_DEV "/dev/input/event2"  // 修改为实际设备节点
```

### 回调间隔配置

修改`CALLBACK_INTERVAL_MS`宏定义调整处理频率：

```c
#define CALLBACK_INTERVAL_MS 10  // 回调处理间隔（毫秒）
```

## 使用示例

### 基本使用

```c
#include "touchboard.h"

// 自定义回调函数
void my_callback(int x, int y) {
    printf("触摸坐标: X=%d, Y=%d\n", x, y);
}

int main() {
    // 设置回调函数
    set_touch_callback(my_callback);
    
    // 初始化设备
    if (touch_init() != 0) {
        fprintf(stderr, "初始化失败\n");
        return 1;
    }
    
    // 主循环
    while (1) {
        sleep(1);
    }
    
    // 清理资源
    touch_exit();
    return 0;
}
```

## 技术细节

### 输入事件处理

程序处理以下类型的输入事件：
- `EV_REL`：相对坐标事件（触摸板移动）
- `EV_KEY`：按键事件（触摸状态）
- `EV_SYN`：同步事件（数据包结束）

### 线程同步机制

- **互斥锁**：保护共享的坐标数据
- **条件变量**：实现线程间的事件通知
- **定时等待**：确保回调处理的固定频率

### 错误处理

- 设备打开失败时立即返回错误
- 读取失败时打印错误信息但继续运行
- 线程创建失败时清理已分配资源

## 故障排除

### 设备权限问题

如果遇到"无法打开触摸设备"错误，确保当前用户有访问输入设备的权限：

```bash
# 查看输入设备
ls -l /dev/input/

# 添加用户到input组（需要root权限）
sudo usermod -a -G input $USER
```

### 设备节点查找

使用以下命令查找正确的触摸板设备节点：

```bash
# 列出所有输入设备
cat /proc/bus/input/devices

# 或使用evtest工具
evtool /dev/input/eventX  # 替换X为设备编号
```

## 开发说明

### 代码风格

- 使用清晰的变量命名和注释
- 遵循Linux内核编程规范
- 保持函数职责单一

### 扩展建议

1. **添加事件过滤**：减少不必要的回调调用
2. **实现手势识别**：基于坐标序列识别特定手势
3. **支持多点触摸**：扩展数据结构支持多点输入
4. **添加配置文件**：支持运行时配置参数

## 许可证

本项目采用MIT许可证。详见LICENSE文件。

## 贡献

欢迎提交Issue和Pull Request来改进这个项目。

## 联系方式

如有问题或建议，请联系项目维护者。

---

*最后更新: 2025-11-13*