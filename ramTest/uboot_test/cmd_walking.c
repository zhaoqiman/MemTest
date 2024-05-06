/*
	自定义 u-boot 指令 walking
	用法： walking [start_addr] [test_size]
	示例： walking 00ff0000 1024
	指定起始地址和测试大小，对指定内存区域进行 walking 算法测试
 */
// ulong 与系统数据位宽一致

#include <command.h>
#include <common.h>

int do_walking(struct cmd_tbl *cmdtp, int flag, int argc, char *argv[])
{
	unsigned long *start_addr = (unsigned long *)0x80000;
	int test_size = 1024;

	/* 判断参数个数 */
	if (argc > 3)
	{
		printf("Usage: walking [start_addr] [test_size]\n");
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

	printf("walking test start at 0x%lx, size %d\n", (unsigned long)start_addr, test_size);

	unsigned long *ptr = start_addr;
    unsigned long* check_ptr = start_addr;
	unsigned long *end_addr = start_addr + test_size / sizeof(unsigned long);

	int tmp;
	int i;
	int BIT_WIDTH = sizeof(unsigned long) * 8;

    // 全 0
	for(ptr = start_addr; ptr < end_addr; ptr++){
		*ptr = 0;
	}

    for(ptr = start_addr; ptr < end_addr; ptr++){
        // 首 bit 写 1
        *ptr = 0x1LU;
        for(i = 0; i < BIT_WIDTH; i++){
            // 本宽度内读出验证
            if(*ptr != (0x1LU << i)){
                printf("Walking fail at Address: 0x%lx\n, value: 0x%lx\n", (unsigned long)ptr, *ptr);
                return -1;
            }
            // 其余所有位置读出验证是否为0
            for(check_ptr = start_addr; check_ptr < end_addr; check_ptr++){
                if(check_ptr != ptr && *check_ptr != 0){
                    printf("Walking fail at Address: 0x%lx\n, value: 0x%lx\n", (unsigned long)check_ptr, *check_ptr);
                    return -1;
                }
            }
            // 移位
            *ptr <<= 1;
        }
    }
	
	printf("walking test pass.\n");
	return 0;
}

U_BOOT_CMD(
	walking, 3, 0, do_walking,
	"User-defined command: walking [start_addr] [test_size]\n",
	"Implemented in the walking.c file\n");
