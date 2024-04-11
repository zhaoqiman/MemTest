CC = gcc
CFLAGS = -Wall -Wextra -g

# 编译目标文件
OBJ = dev_main.o mem_test.o

# 默认目标
all: dev_main

# 生成可执行文件
dev_main: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

# 编译 dev_main.c 文件为 dev_main.o 目标文件
dev_main.o: dev_main.c mem_test.h
	$(CC) $(CFLAGS) -c dev_main.c

# 编译 mem_test.c 文件为 mem_test.o 目标文件
mem_test.o: mem_test.c mem_test.h
	$(CC) $(CFLAGS) -c mem_test.c

# 清理编译生成的文件
clean:
	rm -f dev_main $(OBJ)
