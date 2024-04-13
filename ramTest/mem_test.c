// zhaoqiman
// 实现 Checkerboard，March C - 算法
#include "mem_test.h"

// A即 1010 5即 0101；	4*16 = 64bit
static const ulong kCheckerPattern = 0xAAAAAAAAAAAAAAAA;
static const ulong kAntiCheckerPattern = 0x5555555555555555;

ErrorInfo error_info;


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
			error_info.addr = (ulong)addr;
			error_info.write_val = 0xffffffffffffffffLU << j;
			error_info.read_val = *addr;
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
			error_info.addr = (ulong)addr;
			error_info.write_val = 0xffffffffffffffffLU >> j;
			error_info.read_val = *addr;
			return TEST_FAIL;
		}
		*addr &= ~(0x1 << i); // 将第j位置0
	}
	return TEST_PASS;
}


// Checkerboard algorithm RAM test
int CheckerboardTest(ulong* start_addr, int test_size)
{
	ulong* end_addr = start_addr + test_size/BIT_WIDTH*8;
	ulong* ptr = start_addr;
	ulong* ptr_next = ptr + 1;

	for(ptr = start_addr; ptr < end_addr; ptr+=2){
		ptr_next = ptr + 1;
		// 写入
		*ptr = kCheckerPattern;
		*ptr_next = kAntiCheckerPattern;
		// 读出验证
		if(*ptr != kCheckerPattern){
			error_info.addr = (ulong)ptr;
			error_info.write_val = kCheckerPattern;
			error_info.read_val = *ptr;
			return TEST_FAIL;
		}
		if(*ptr_next != kAntiCheckerPattern){
			error_info.addr = (ulong)ptr_next;
			error_info.write_val = kAntiCheckerPattern;
			error_info.read_val = *ptr_next;
			return TEST_FAIL;
		}
	}
	return TEST_PASS;
}


// Walking algorithm RAM test
int WalkingTest(ulong* start_addr, int test_size){
	int i;
	ulong* end_addr = start_addr + test_size/BIT_WIDTH*8;
	ulong* ptr = start_addr;
	
	// 全 0
	for(ptr = start_addr; ptr < end_addr; ptr++){
		*ptr = 0;
	}

	for(ptr = start_addr; ptr < end_addr; ptr++){
		// 首 bit 写 1
		*ptr = 0x1LU;
		for(i = 0; i < BIT_WIDTH; i++){
			// 读出验证
			if(*ptr != (0x1LU << i)){
				error_info.addr = (ulong)ptr;
				error_info.write_val = 0x1LU << i;
				error_info.read_val = *ptr;
				return TEST_FAIL;
			}
			// 移位
			*ptr <<= 1;
		}
	}
	return TEST_PASS;
}


// March C- algorithm RAM test 
// Args:	
//		start_addr: 待测内存起始地址
//		test_size: 待测内存大小(Byte)
int MarchCTest(ulong* start_addr, int test_size){
	int result = TEST_FAIL;
	ulong* end_addr = start_addr + test_size/BIT_WIDTH*8;
	ulong* ptr = start_addr;

	// 1. 正序 write 0
	for(ptr = start_addr; ptr < end_addr; ptr++){
		*ptr = 0;
	}
	result = TEST_PASS;

	// 2. 正序 read 0 write 1
	for(ptr = start_addr; ptr < end_addr && result == TEST_PASS; ptr++){
		result = R0W1(ptr,false);
	}

	// 3. 反序 read 1 write 0
	for(ptr = end_addr-1; ptr >= start_addr && result == TEST_PASS; ptr--){
		result = R1W0(ptr, true);
	}

	// 4. 反序 read 0 write 1
	for(ptr = end_addr-1; ptr >= start_addr && result == TEST_PASS; ptr--){
		result = R0W1(ptr, true);
	}

	// 5. 正序 read 1 write 0 read 0
	for(ptr = start_addr; ptr < end_addr && result == TEST_PASS; ptr++){
		result = R1W0(ptr, false);
		if(*ptr != 0){
			error_info.addr = (ulong)ptr;
			error_info.write_val = 0;
			error_info.read_val = *ptr;
			return TEST_FAIL;
		}
	}

	return result;
}

