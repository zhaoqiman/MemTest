// zhaoqiman
// 实现 March C - 算法
// 使用 /dev/mem 设备 实现对 RAM 的读写


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

// ulong 与系统数据位宽一致
typedef unsigned long ulong;
// 位宽常数	/bit
#define BIT_WIDTH 8*sizeof(ulong)
#define TEST_FAIL 0
#define TEST_PASS 1
static ulong errorAddr = 0;
static int errorType = -1;


// FATAl宏定义，用于打印错误信息
#define FATAL do { fprintf(stderr, "Error at line %d, file %s (%d) [%s]\n", \
  __LINE__, __FILE__, errno, strerror(errno)); exit(1); } while(0)
 
#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)
 

// 对ulong逐个bit 读0 写1 
static int R0W1(volatile ulong* addr,bool reverse = false){
	int tmp;
	int i;
	if(reverse == false){	// 正序
		for(i = BIT_WIDTH-1; i >= 0; i--){
			tmp = (((*addr) >> i) & 0x1); // 取出第i位
			if(tmp != 0){
				errorAddr = (ulong)addr;
				errorType = 0;
				return TEST_FAIL;
			}
			*addr |= (0x1 << i); // 将第i位置1
		}
	}
	else{	// 反序
		for(i = 0; i < BIT_WIDTH; i++){
			tmp = (((*addr) >> i) & 0x1); // 取出第i位
			if(tmp != 0){
				errorAddr = (ulong)addr;
				errorType = 0;
				return TEST_FAIL;
			}
			*addr |= (0x1 << i); // 将第i位置1
		}
	}
	return TEST_PASS;
}
// 对ulong逐个bit 读1 写0
static int R1W0(volatile ulong* addr, bool reverse = false){
	int tmp;
	int i;
	if(reverse == false){	// 正序
		for(i = BIT_WIDTH-1; i >= 0; i--){
			// R1
			tmp = (((*addr) >> i) & 0x1); // 取出第i位
			if(tmp != 1){
				errorAddr = (ulong)addr;
				errorType = 1;
				return TEST_FAIL;
			}
			// W0
			*addr &= ~(0x1 << i); // 将第i位置0
		}
	}
	else{
		for(i = 0; i < BIT_WIDTH; i++){
			// R1
			tmp = (((*addr) >> i) & 0x1); // 取出第i位
			if(tmp != 1){
				errorAddr = (ulong)addr;
				errorType = 1;
				return TEST_FAIL;
			}
			// W0
			*addr &= ~(0x1 << i); // 将第i位置0
		}
	}
	return TEST_PASS;
}




// March C- algorithm RAM test 
// Args:	
//		startAddr: the start address of the memory to be tested
//		testSize: the size of the memory to be tested (Byte)
static int MarchCTest(ulong* startAddr, int testSize, bool printStep = false){
	int result = TEST_FAIL;
	ulong* endAddr = startAddr + testSize/BIT_WIDTH*8;
	ulong* ptr = startAddr;

	// 1. 正序 write 0
	for(ptr = startAddr; ptr < endAddr; ptr++){
		*ptr = 0;
	}
	result = TEST_PASS;
	
	if(printStep){
		printf("Step 1: 正序 write 0. Over.\n");
	}

	// 2. 正序 read 0 write 1
	for(ptr = startAddr; ptr < endAddr && result == TEST_PASS; ptr++){
		result = R0W1(ptr,false);
	}

	if(printStep){
		printf("Step 2: 正序 read 0 write 1. Over.\n");
	}

	// 3. 反序 read 1 write 0
	for(ptr = endAddr-1; ptr >= startAddr && result == TEST_PASS; ptr--){
		result = R1W0(ptr, true);
	}
	if(printStep){
		printf("Step 3: 反序 read 1 write 0. Over.\n");
	}

	// 4. 反序 read 0 write 1
	for(ptr = endAddr-1; ptr >= startAddr && result == TEST_PASS; ptr--){
		result = R0W1(ptr, true);
	}
	if(printStep){
		printf("Step 4: 反序 read 0 write 1. Over.\n");
	}
	// 5. 正序 read 1 write 0
	for(ptr = startAddr; ptr < endAddr && result == TEST_PASS; ptr++){
		result = R1W0(ptr, false);
	}
	if(printStep){
		printf("Step 5: 正序 read 1 write 0. Over.\n");
	}

	return result;
}



int main(int argc, char **argv) {
    int fd;
    void *map_base;
	ulong* virt_addr;
	unsigned long read_result, writeval;
	off_t target;
	int access_type = 'w';
	
	if(argc < 2) { //若参数个数少于两个则打印此工具的使用方法
		fprintf(stderr, "\nUsage:\t%s { startAddr } {testSize} ]\n"
			"\tstartAddr : start address to test\n"
			"\ttestSize    : the size of the memory to be tested (Byte)\n",
			argv[0]);
		exit(1);
	}

	ulong startAddr;
	int testSize;
	// 第一个参数 startAddr 为要测试的内存的起始地址
	startAddr = strtoul(argv[1], 0, 0);	// 将字符串转换为无符号长整型
	// 第二个参数 testSize 为要测试的内存的大小 str -> int
	testSize = atoi(argv[2]);
	
	// 1. 打开 /dev/mem 设备
    if((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1) FATAL;
    printf("/dev/mem opened.\n"); 
    fflush(stdout);
    
    //2. 将内核空间映射到用户空间
    map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, startAddr & ~MAP_MASK);
    if(map_base == (void *) -1) FATAL;
    printf("Memory mapped at address %p.\n", map_base); 
    fflush(stdout);
    
    virt_addr = (ulong*)map_base + (startAddr & MAP_MASK);
    
    // 根据 testAddr 和 testSize 进行测试
	// 借虚拟地址完成 ram 的读写
	if(argc > 1) {
		int result = MarchCTest(virt_addr, testSize);

		if(result = TEST_PASS){
			printf("Test over, no error.\n");
		}
		else{
			printf("Error at Address 0x%X， ulong value 0x%X, errorType %d\n", errorAddr, *((unsigned long *) errorAddr), errorType);
		}
		fflush(stdout);
	}
	
	if(munmap(map_base, MAP_SIZE) == -1) FATAL;
    close(fd);
    return 0;
}