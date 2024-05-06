/*
	自定义 u-boot 指令 march
	用法： march [start_addr] [test_size]
	示例： march 00ff0000 1024
	指定起始地址和测试大小，对指定内存区域进行 march C -算法测试
 */
// ulong 与系统数据位宽一致

#include <command.h>
#include <common.h>

int do_march(struct cmd_tbl *cmdtp, int flag, int argc, char *argv[])
{
	unsigned long *start_addr = (unsigned long *)0x80000;
	int test_size = 1024;

	/* 判断参数个数 */
	if (argc > 3)
	{
		printf("Usage: march [start_addr] [test_size]\n");
		return 0;
	}
	else if (argc > 2)
	{
		// 第二个参数 test_size 为要测试的内存的大小 str -> int
		// 交叉编译器不支持 atoi
		test_size = 0;
		char *str = argv[2];
		while (*str)
		{
			if (*str >= '0' && *str <= '9')
			{
				test_size = test_size * 10 + (*str - '0');
			}
			else
			{
				break;
			}
			str++;
		}
		if (test_size <= 0)
		{
			printf("Test size must be greater than 0\n");
			return -1;
		}
	}
	if (argc > 1)
	{
		// 第一个参数 start_addr 为要测试的内存的起始地址 str -> long
		unsigned long temp_addr = 0;
		char *str = argv[1];
		while (*str)
		{
			if (*str >= '0' && *str <= '9')
			{
				temp_addr = temp_addr * 16 + (*str - '0');
			}
			else if (*str >= 'a' && *str <= 'f')
			{
				temp_addr = temp_addr * 16 + (*str - 'a' + 10);
			}
			else if (*str >= 'A' && *str <= 'F')
			{
				temp_addr = temp_addr * 16 + (*str - 'A' + 10);
			}
			else
			{
				break;
			}
			str++;
		}
		if (temp_addr < 0x80000)
		{
			printf("Start address must be greater than 0x80000\n");
			return -1;
		}
		else
		{
			start_addr = (unsigned long *)temp_addr;
		}
	}

	printf("March C- test start at 0x%lx, size %d\n", (unsigned long)start_addr, test_size);

	unsigned long *ptr = start_addr;
	unsigned long *end_addr = start_addr + test_size / sizeof(unsigned long);

	int tmp;
	int i;
	int BIT_WIDTH = sizeof(unsigned long) * 8;

	// 1. UP Write 0
	for (ptr = start_addr; ptr < end_addr; ptr++)
	{
		*ptr = 0;
	}
	// 2. UP read 0 write 1
	for (ptr = start_addr; ptr < end_addr; ptr++)
	{
		for(i = BIT_WIDTH - 1; i >= 0; i--){
			tmp = (((*ptr) >> i) & 0x1LU); // 取出第i位. 
			if(tmp != 0){
				printf("March C- Step 2 fail at Address: 0x%lx\n, value: 0x%lx\n", (unsigned long)ptr, *ptr);
				return -1;
			}
			*ptr = (*ptr | (0x1LU << i)); // 将第i位置1. 
		}
	}
	// 3. DOWN read 1 write 0
	for (ptr = end_addr - 1; ptr >= start_addr; ptr--)
	{
		for(i = 0; i < BIT_WIDTH; i++){
			tmp = (((*ptr) >> i) & 0x1LU); // 取出第i位. 
			if(tmp != 1){
				printf("March C- Step 3 fail at Address: 0x%lx\n, value: 0x%lx\n", (unsigned long)ptr, *ptr);
				return -1;
			}
			*ptr &= ~(0x1LU << i); // 将第i位置0. 
		}
	}
	// 4. DOWN read 0 write 1
	for (ptr = end_addr - 1; ptr >= start_addr; ptr--)
	{
		for(i = BIT_WIDTH - 1; i >= 0; i--){
			tmp = (((*ptr) >> i) & 0x1LU); // 取出第i位. 
			if(tmp != 0){
				printf("March C- Step 4 fail at Address: 0x%lx\n, value: 0x%lx\n", (unsigned long)ptr, *ptr);
				return -1;
			}
			*ptr = (*ptr | (0x1LU << i)); // 将第i位置1. 
		}
	}
	// 5. UP read 1 write 0 read 0
	for (ptr = start_addr; ptr < end_addr; ptr++)
	{
		for(i = 0; i < BIT_WIDTH; i++){
			tmp = (((*ptr) >> i) & 0x1LU); // 取出第i位. 
			if(tmp != 1){
				printf("March C- Step 3 fail at Address: 0x%lx\n, value: 0x%lx\n", (unsigned long)ptr, *ptr);
				return -1;
			}
			*ptr &= ~(0x1LU << i); // 将第i位置0. 
		}
		if(*ptr != 0){
			printf("March C- Step 5 fail at Address: 0x%lx\n, value: 0x%lx\n", (unsigned long)ptr, *ptr);
			return -1;
		}
	}

	printf("March C- test pass.\n");
	return 0;
}

U_BOOT_CMD(
	march, 3, 0, do_march,
	"User-defined command: march [start_addr] [test_size]\n",
	"Implemented in the cmd_march.c file\n");
