/*
	自定义 u-boot 指令 checker
	用法： checker [start_addr] [test_size]
	示例： checker 00ff0000 1024
	指定起始地址和测试大小，对指定内存区域进行 checkerboard 算法测试
 */
// ulong 与系统数据位宽一致

#include <command.h>
#include <common.h>

int do_checker(struct cmd_tbl *cmdtp, int flag, int argc, char *argv[])
{
	unsigned long *start_addr = (unsigned long *)0x80000;
	int test_size = 1024;

	/* 判断参数个数 */
	if (argc > 3)
	{
		printf("Usage: checker [start_addr] [test_size]\n");
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

	printf("checkerboard test start at 0x%lx, size %d\n", (unsigned long)start_addr, test_size);

	unsigned long *ptr = start_addr;
    unsigned long *ptr_next = start_addr + 1;
	unsigned long *end_addr = start_addr + test_size / sizeof(unsigned long);

	int tmp;
	int i;
	int BIT_WIDTH = sizeof(unsigned long) * 8;

    ulong kCheckerPattern = 0xAAAAAAAAAAAAAAAA;
    ulong kAntiCheckerPattern = 0x5555555555555555;

	for(ptr = start_addr; ptr < end_addr; ptr+=2){
		ptr_next = ptr + 1;
		// 写入
		*ptr = kCheckerPattern;
		*ptr_next = kAntiCheckerPattern;
		// 读出验证
		if(*ptr != kCheckerPattern){
            printf("Checkerboard fail at Address: 0x%lx\n, value: 0x%lx\n", (unsigned long)ptr, *ptr);
            return -1;
		}
		if(*ptr_next != kAntiCheckerPattern){
            printf("Checkerboard fail at Address: 0x%lx\n, value: 0x%lx\n", (unsigned long)ptr_next, *ptr_next);
            return -1;
		}
	}
	
	printf("checkerboard test pass.\n");
	return 0;
}

U_BOOT_CMD(
	checker, 3, 0, do_checker,
	"User-defined command: checker [start_addr] [test_size]\n",
	"Implemented in the checker.c file\n");
