#ifndef __MEMTEST_H__
#define __MEMTEST_H__

#include <stdbool.h>
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
// 系统位宽常数	/bit
#define BIT_WIDTH 8*sizeof(ulong)
#define TEST_FAIL 0
#define TEST_PASS 1



typedef struct Error_Info
{
    ulong addr;
    ulong readData;
    ulong writeData;
}ErrorInfo;


// 对ulong逐个bit 读0 写1 
int R0W1(volatile ulong* addr,bool reverse);
int R1W0(volatile ulong* addr, bool reverse);
// March C- algorithm RAM test 
// Args:	
//		startAddr: the start address of the memory to be tested
//		testSize: the size of the memory to be tested (Byte)
int MarchCTest(ulong* startAddr, int testSize);
int CheckerboardTest(ulong* startAddr, int testSize);

#endif // __MEMTEST_H__