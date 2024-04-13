// 调用 mem_test.c 中测试函数
// 在用户态使用 /mem/dev 读写内存

#include "mem_test.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/mman.h>


#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

// FATAl宏定义，用于打印错误信息
#define FATAL do { fprintf(stderr, "Error at line %d, file %s (%d) [%s]\n", \
  __LINE__, __FILE__, errno, strerror(errno)); exit(1); } while(0)

extern ErrorInfo error_info;

int main(int argc, char **argv) {
    int fd;
    void *map_base;
	ulong* virt_start_addr;
	//unsigned long read_result, writeval;
	//off_t target;
	//int access_type = 'w';
	
	if(argc < 2) { //若参数个数少于两个则打印此工具的使用方法
		fprintf(stderr, "\nUsage:\t%s { start_addr } {test_size} ]\n"
			"\tstartAddr : start address to test\n"
			"\ttestSize    : the size of the memory to be tested (Byte)\n",
			argv[0]);
		exit(1);
	}

	ulong start_addr;
	int test_size;
	// 第一个参数 start_addr 为要测试的内存的起始地址
	start_addr = strtoul(argv[1], 0, 0);	// 将字符串转换为无符号长整型
	// 第二个参数 test_size 为要测试的内存的大小 str -> int
	test_size = atoi(argv[2]);
	
	// 1. 打开 /dev/mem 设备
    if((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1) FATAL;
    printf("/dev/mem opened.\n"); 
    fflush(stdout);
    
    //2. 将内核空间映射到用户空间
    map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, start_addr & ~MAP_MASK);
    if(map_base == (void *) -1) FATAL;
    printf("Memory mapped at address %p.\n", map_base); 
    fflush(stdout);
    
    virt_start_addr = (ulong*)map_base + (start_addr & MAP_MASK);
    
    // 根据 testAddr 和 test_size 进行测试
	// 借虚拟地址完成 ram 的读写
	if(argc > 1) {
		int march_result = MarchCTest(virt_start_addr, test_size);
        int checker_result = CheckerboardTest(virt_start_addr, test_size);
		int walking_result = WalkingTest(virt_start_addr, test_size);
		if(march_result&&checker_result&&walking_result){
			printf("Test over, no error.\n");
		}
		else{
			ulong real_error_addr = error_info.addr + (ulong)(virt_start_addr - (start_addr & MAP_MASK));
			printf("Error at Address 0x%lX， ulong write value 0x%lX, ulong read value  0x%lX\n", real_error_addr, error_info.write_val, error_info.read_val);
			if(march_result == TEST_FAIL) printf("March C- test failed.\n");
			if(checker_result == TEST_FAIL) printf("Checkerboard test failed.\n");
			if(walking_result == TEST_FAIL) printf("Walking test failed.\n");
		}
		fflush(stdout);
	}
	
	if(munmap(map_base, MAP_SIZE) == -1) FATAL;
    close(fd);
    return 0;
}