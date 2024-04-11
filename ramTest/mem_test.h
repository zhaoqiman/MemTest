#ifndef __MEM_TEST_H__
#define __MEM_TEST_H__

#include <stdio.h>
#include <stdbool.h>

// ulong 与系统数据位宽一致
typedef unsigned long ulong;
// 系统位宽常数	/bit
#define BIT_WIDTH (int)(8*sizeof(ulong))
#define TEST_FAIL 0
#define TEST_PASS 1


typedef struct Error_Info
{
    ulong addr;
    ulong read_val;
    ulong write_val;
}ErrorInfo;


// 对ulong逐个bit 读0 写1 
// Args:
//		addr: 要测试的地址
//		reverse: 是否逆序读写
int R0W1(volatile ulong* addr,bool reverse);
int R1W0(volatile ulong* addr, bool reverse);

// March C- algorithm RAM test 
// Args:	
//		start_addr: 待测内存起始地址
//		test_size: 待测内存大小(Byte)
int MarchCTest(ulong* start_addr, int test_size);

// Checkerboard algorithm RAM test
int CheckerboardTest(ulong* start_addr, int test_size);

#endif // __MEM_TEST_H__