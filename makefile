# 定义编译器
CC = /home/bianxindong/001-US_1/host/bin/aarch64-buildroot-linux-gnu-gcc

# 定义编译器标志
CFLAGS = -Wall -g 

# 定义源文件目录
SRCDIR = .

# 定义头文件目录
INCDIR = .

# 定义目标文件目录
OBJDIR = obj

# 定义源文件
SRCS = $(wildcard $(SRCDIR)/*.c)

# 定义目标文件
OBJS = $(SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

# 定义可执行文件名
TARGET = kb_control

# 创建目标文件目录
$(shell mkdir -p $(OBJDIR))

# 默认目标
all: $(TARGET)

# 链接目标文件生成可执行文件
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# 编译源文件生成目标文件
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

# 清理编译生成的文件
clean:
	rm -rf $(OBJDIR) $(TARGET)

.PHONY: all clean