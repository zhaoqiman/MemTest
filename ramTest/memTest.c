// zhaoqiman
// 实现 March C - 算法
// 使用 /dev/mem 设备 实现对 RAM 的读写
#include "memTest.h"


// A即 1010 5即 0101
static const ulong checkerPattern = 0xAAAAAAAAAAAAAAAA; // 4*16 = 64bit
static const ulong antiCheckerPattern = 0x5555555555555555;
extern ErrorInfo errorInfo;

// 对ulong逐个bit 读0 写1 
int R0W1(volatile ulong* addr,bool reverse){
	int tmp;
	int i,j;

	for(i = 0; i < BIT_WIDTH; i++){
		if(reverse){
			j = i;
		}
		else{
			j = BIT_WIDTH-1-i;
		}
		tmp = (((*addr) >> j) & 0x1); // 取出第j位
		if(tmp != 0){
			errorInfo.addr = (ulong)addr;
			errorInfo.writeData = 0xffffffffffffffffLU << j;
			errorInfo.readData = *addr;
			return TEST_FAIL;
		}
		*addr |= (0x1LU << j); // 将第j位置1
	}
	return TEST_PASS;
}
// 对ulong逐个bit 读1 写0
int R1W0(volatile ulong* addr, bool reverse){
	int tmp;
	int i,j;


	for(i = 0; i < BIT_WIDTH; i++){
		if(reverse){
			j = i;
		}
		else{
			j = BIT_WIDTH-1-i;
		}
		tmp = (((*addr) >> j) & 0x1); // 取出第j位
		if(tmp != 1){
			errorInfo.addr = (ulong)addr;
			errorInfo.writeData = 0xffffffffffffffffLU >> j;
			errorInfo.readData = *addr;
			return TEST_FAIL;
		}
		*addr &= ~(0x1 << i); // 将第j位置0
	}
	return TEST_PASS;
}


// March C- algorithm RAM test 
// Args:	
//		startAddr: the start address of the memory to be tested
//		testSize: the size of the memory to be tested (Byte)
int MarchCTest(ulong* startAddr, int testSize){
	int result = TEST_FAIL;
	ulong* endAddr = startAddr + testSize/BIT_WIDTH*8;
	ulong* ptr = startAddr;

	// 1. 正序 write 0
	for(ptr = startAddr; ptr < endAddr; ptr++){
		*ptr = 0;
	}
	result = TEST_PASS;

	// 2. 正序 read 0 write 1
	for(ptr = startAddr; ptr < endAddr && result == TEST_PASS; ptr++){
		result = R0W1(ptr,false);
	}

	// 3. 反序 read 1 write 0
	for(ptr = endAddr-1; ptr >= startAddr && result == TEST_PASS; ptr--){
		result = R1W0(ptr, true);
	}

	// 4. 反序 read 0 write 1
	for(ptr = endAddr-1; ptr >= startAddr && result == TEST_PASS; ptr--){
		result = R0W1(ptr, true);
	}

	// 5. 正序 read 1 write 0
	for(ptr = startAddr; ptr < endAddr && result == TEST_PASS; ptr++){
		result = R1W0(ptr, false);
	}

	return result;
}

int CheckerboardTest(ulong* startAddr, int testSize)
{
	int result = TEST_FAIL;
	ulong* ptr = startAddr;

	while(testSize > 1){
		// 写入
		*ptr = checkerPattern;
		*(ptr+1) = antiCheckerPattern;
		// 读出验证
		if(*ptr != checkerPattern){
			errorInfo.addr = (ulong)ptr;
			errorInfo.writeData = checkerPattern;
			errorInfo.readData = *ptr;
			return TEST_FAIL;
		}
		if(*(ptr+1) != antiCheckerPattern){
			errorInfo.addr = (ulong)(ptr+1);
			errorInfo.writeData = antiCheckerPattern;
			errorInfo.readData = *ptr;
			return TEST_FAIL;
		}
		ptr += 2;
		testSize -= 2;
	}
	return TEST_PASS;
}


