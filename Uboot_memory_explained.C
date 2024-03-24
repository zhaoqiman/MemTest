/*
	u-boot/post/drivers/memory.c
	U-Boot 上电自检内存代码
	自加注释、翻译等
*/

#include <common.h>

/* Memory test	内存测试
 *
 * General observations:	一般观察：
 * o The recommended test sequence is to test the data lines: if they are
 *   broken, nothing else will work properly.  Then test the address
 *   lines.  Finally, test the cells in the memory now that the test
 *   program knows that the address and data lines work properly.
 *   This sequence also helps isolate and identify what is faulty.
 * 		建议的测试顺序 		
 * 			1.测试数据线：如果数据线不行，其他任何东西都不会正常工作。
 * 			2.地址线。
 * 			3.测试内存中的单元，此时测试程序知道地址和数据线工作正常。
 * 		此顺序也有助于隔离和识别故障。
 *
 * o For the address line test, it is a good idea to use the base
 *   address of the lowest memory location, which causes a '1' bit to
 *   walk through a field of zeros on the address lines and the highest
 *   memory location, which causes a '0' bit to walk through a field of
 *   '1's on the address line.
 * 		对地址线测试：最好使用最低内存位置的基地址，这会导致'1'位在地址线上的零字段中移动，
 *					以及最高内存位置的基地址，这会导致'0'位在地址线上的'1'字段中移动。
 *		又称 "走比特测试"
 * o Floating buses can fool memory tests if the test routine writes
 *   a value and then reads it back immediately.  The problem is, the
 *   write will charge the residual capacitance on the data bus so the
 *   bus retains its state briefely.  When the test program reads the
 *   value back immediately, the capacitance of the bus can allow it
 *   to read back what was written, even though the memory circuitry
 *   is broken.  To avoid this, the test program should write a test
 *   pattern to the target location, write a different pattern elsewhere
 *   to charge the residual capacitance in a differnt manner, then read
 *   the target location back.
 * 		
 * 浮动总线问题：
 * 		测试软件写入并立即从相同地址读出一个值时，
 * 		如果数据线上存在电容特性，写操作会给数据线上的电容充电，总线会短暂的保持它的状态。
 * 		当测试软件读操作时，总线会返回刚写入的值，即使实际上该数据并没有正确地被写入存储单元。
 * 规避方法：
 * 		write A to address X;
		write inversion of A to address Y;
		read value from address X;
 *
 * o Always read the target location EXACTLY ONCE and save it in a local
 *   variable.  The problem with reading the target location more than
 *   once is that the second and subsequent reads may work properly,
 *   resulting in a failed test that tells the poor technician that
 *   "Memory error at 00000000, wrote aaaaaaaa, read aaaaaaaa" which
 *   doesn't help him one bit and causes puzzled phone calls.  Been there,
 *   done that.
 *	永远只读取目标位置一次，并将其保存在一个局部变量中。
 *	多次读取目标位置的问题是，第二次和后续的读取可能正常，导致测试失败，
 *  防止出现报错如“内存错误在00000000处，写入aaaaaaa，读取aaaaaaa”，没有任何实际帮助。
 * 
 * Data line test:
 * ---------------
 * This tests data lines for shorts and opens by forcing adjacent data
 * to opposite states. Because the data lines could be routed in an
 * arbitrary manner the must ensure test patterns ensure that every case
 * is tested. By using the following series of binary patterns every
 * combination of adjacent bits is test regardless of routing.
 * 
 * 数据线测试：
 * 		通过强制相邻数据到相反状态，测试数据线的短路和断路。
 * 		因为数据线可能以任意方式路由，所以必须确保测试样板确保测试每种情况。
 * 		通过使用以下一系列二进制样板，测试每种相邻位的组合，无论路由如何。
 * 
 *     ...101010101010101010101010
 *     ...110011001100110011001100
 *     ...111100001111000011110000
 *     ...111111110000000011111111
 *
 * Carrying this out, gives us six hex patterns as follows:
 * 进而，我们得到六个十六进制样板如下：
 *
 *     0xaaaaaaaaaaaaaaaa
 *     0xcccccccccccccccc
 *     0xf0f0f0f0f0f0f0f0
 *     0xff00ff00ff00ff00
 *     0xffff0000ffff0000
 *     0xffffffff00000000
 *
 * 
 * To test for short and opens to other signals on our boards, we
 * simply test with the 1's complemnt of the paterns as well, resulting
 * in twelve patterns total.
 * 为了测试板上其他信号的短路和断路，我们也测试了样板的补码，总共得到12个样板。
 *
 * After writing a test pattern. a special pattern 0x0123456789ABCDEF is
 * written to a different address in case the data lines are floating.
 * Thus, if a byte lane fails, you will see part of the special
 * pattern in that byte lane when the test runs.  For example, if the
 * xx__xxxxxxxxxxxx byte line fails, you will see aa23aaaaaaaaaaaa
 * (for the 'a' test pattern).
 * 写入测试样板后，特殊样板0x0123456789ABCDEF被写入到不同的地址，以防数据线浮动。
 * 因此，如果一个字节线失败，当测试运行时，你会看到特殊样板的一部分在该字节线上。
 * 例如，如果xx__xxxxxxxxxxxx字节线失败，你会看到aa23aaaaaaaaaaaa（对于'a'测试样板）。
 *
 * Address line test:
 * 地址线测试：
 * ------------------
 *  This function performs a test to verify that all the address lines
 *  hooked up to the RAM work properly.  If there is an address line
 *  fault, it usually shows up as two different locations in the address
 *  map (related by the faulty address line) mapping to one physical
 *  memory storage location.  The artifact that shows up is writing to
 *  the first location "changes" the second location.
 * 此函数执行一个测试，以验证所有连接到RAM的地址线是否正常工作。
 * 如有地址线故障，通常表现为地址映射中的两个不同位置（由故障地址线相关）映射到一个物理内存存储位置。
 * 表现即，写入第一个位置“改变”了第二个位置。
 *
 * To test all address lines, we start with the given base address and
 * xor the address with a '1' bit to flip one address line.  For each
 * test, we shift the '1' bit left to test the next address line.
 * 为了测试所有地址线，我们从给定的基地址开始，将地址与'1'位异或，以翻转一个地址线。
 * 对于每个测试，我们将'1'位左移，以测试下一个地址线。
 * 
 * In the actual code, we start with address sizeof(ulong) since our
 * test pattern we use is a ulong and thus, if we tried to test lower
 * order address bits, it wouldn't work because our pattern would
 * overwrite itself.
 * 在实际代码中，我们从地址sizeof(ulong)开始，因为我们使用的测试样板是一个ulong，
 * 因此，如果我们尝试测试低阶地址位，它不会工作，因为我们的样板会覆盖自己。
 *
 * Example for a 4 bit address space with the base at 0000:
 * 4位地址空间的示例，基地址为0000：
 *   0000 <- base
 *   0001 <- test 1
 *   0010 <- test 2
 *   0100 <- test 3
 *   1000 <- test 4
 * Example for a 4 bit address space with the base at 0010:
 * 4位地址空间的示例，基地址为0010：
 *   0010 <- base
 *   0011 <- test 1
 *   0000 <- (below the base address, skipped)
 *   0110 <- test 2
 *   1010 <- test 3
 *
 * The test locations are successively tested to make sure that they are
 * not "mirrored" onto the base address due to a faulty address line.
 * Note that the base and each test location are related by one address
 * line flipped.  Note that the base address need not be all zeros.
 * 测试位置被连续测试，以确保它们不会由于故障地址线而“镜像”到基地址。
 * 注意，基地址和每个测试位置都由一个地址线翻转相关。
 * 注意，基地址一定不是全零。
 *
 * Memory tests 1-4:
 * 内存测试1-4：
 * -----------------
 * These tests verify RAM using sequential writes and reads
 * to/from RAM. There are several test cases that use different patterns to
 * verify RAM. Each test case fills a region of RAM with one pattern and
 * then reads the region back and compares its contents with the pattern.
 * The following patterns are used:
 * 这些测试使用顺序写入和读取来验证RAM。有几个测试用例使用不同的样板来验证RAM。
 * 每个测试用例都使用一个样板填充RAM的一个区域，然后读取该区域并将其内容与样板进行比较。
 * 使用以下样板：
 *
 *  1a) zero pattern (0x00000000)	零样板
 *  1b) negative pattern (0xffffffff)	负样板
 *  1c) checkerboard pattern (0x55555555)	棋盘样板
 *  1d) checkerboard pattern (0xaaaaaaaa)	棋盘样板
 *  2)  bit-flip pattern ((1 << (offset % 32))	位翻转样板
 *  3)  address pattern (offset)	地址样板
 *  4)  address pattern (~offset)	地址样板的补码
 *
 * Being run in normal mode, the test verifies only small 4Kb
 * regions of RAM around each 1Mb boundary. For example, for 64Mb
 * RAM the following areas are verified: 0x00000000-0x00000800,
 * 0x000ff800-0x00100800, 0x001ff800-0x00200800, ..., 0x03fff800-
 * 0x04000000. If the test is run in slow-test mode, it verifies
 * the whole RAM.
 * 在正常样板下运行，测试只验证每个1Mb边界周围的小4Kb区域。
 * 例如，对于64Mb RAM，将验证以下区域：0x00000000-0x00000800，
 * 0x000ff800-0x00100800，0x001ff800-0x00200800，...，0x03fff800-0x04000000。
 * 如果测试在慢测试样板下运行，它将验证整个RAM。
 */

#include <post.h>
#include <watchdog.h>

#if CONFIG_POST & (CONFIG_SYS_POST_MEMORY | CONFIG_SYS_POST_MEM_REGIONS)

DECLARE_GLOBAL_DATA_PTR;

/*
 * Define INJECT_*_ERRORS for testing error detection in the presence of
 * _good_ hardware.
 */
#undef  INJECT_DATA_ERRORS
#undef  INJECT_ADDRESS_ERRORS

#ifdef INJECT_DATA_ERRORS
#warning "Injecting data line errors for testing purposes"
#endif

#ifdef INJECT_ADDRESS_ERRORS
#warning "Injecting address line errors for testing purposes"
#endif


/*
 * This function performs a double word move from the data at
 * the source pointer to the location at the destination pointer.
 * This is helpful for testing memory on processors which have a 64 bit
 * wide data bus.
 * 此函数实现了一个双字移动，从源指针的数据到目的指针的位置。
 * 对于测试64位宽数据总线的处理器上的内存非常有用。
 *
 * On those PowerPC with FPU, use assembly and a floating point move:
 * this does a 64 bit move.
 * 对于那些带有FPU的PowerPC处理器，使用汇编和浮点移动： 可以实现64位移动。
 * 
 * For other processors, let the compiler generate the best code it can.
 * 对于其他处理器，让编译器生成最好的代码。
 */
static void move64(const unsigned long long *src, unsigned long long *dest)
{
	*dest = *src;
}

/*
 * This is 64 bit wide test patterns.  Note that they reside in ROM
 * (which presumably works) and the tests write them to RAM which may
 * not work.
 * 这是64位宽的测试样板。注意它们驻留在ROM中（假定工作正常），测试将它们写入到RAM中，可能不工作。
 *
 * The "otherpattern" is written to drive the data bus to values other
 * than the test pattern.  This is for detecting floating bus lines.
 * "otherpattern"被写入以驱动数据总线到除测试样板外的其他值。这是用于检测浮动总线。
 */
const static unsigned long long pattern[] = {
	0xaaaaaaaaaaaaaaaaULL,	// 二进制即 1010 1010 ... ...
	0xccccccccccccccccULL,	// 二进制即 1100 1100 ... ...
	0xf0f0f0f0f0f0f0f0ULL,  // 二进制即 1111 0000 ... ...
	0xff00ff00ff00ff00ULL,
	0xffff0000ffff0000ULL,
	0xffffffff00000000ULL,	
	0x00000000ffffffffULL,	//六个反码
	0x0000ffff0000ffffULL,
	0x00ff00ff00ff00ffULL,
	0x0f0f0f0f0f0f0f0fULL,
	0x3333333333333333ULL,
	0x5555555555555555ULL
};
const unsigned long long otherpattern = 0x0123456789abcdefULL;


// 	数据线测试 
//	通过强制相邻数据到相反状态，测试数据线的短路和断路。
// 64位系统中 unsigned long long 占64bit，即8字节. 
static int memory_post_dataline(unsigned long long * pmem)
{
	unsigned long long temp64 = 0;
	int num_patterns = ARRAY_SIZE(pattern);
	int i;
	unsigned int hi, lo, pathi, patlo;
	int ret = 0;

	for ( i = 0; i < num_patterns; i++) {
		move64(&(pattern[i]), pmem++);
		/*
		 * Put a different pattern on the data lines: otherwise they
		 * may float long enough to read back what we wrote.
		 * 在数据线上放置不同的样板：否则它们可能会浮动过长，读回我们写入的内容。
		 */
		move64(&otherpattern, pmem--);
		move64(pmem, &temp64);

#ifdef INJECT_DATA_ERRORS
		temp64 ^= 0x00008000;
#endif

		// temp64 是读回的数据
		if (temp64 != pattern[i]){
			pathi = (pattern[i]>>32) & 0xffffffff;
			patlo = pattern[i] & 0xffffffff;

			hi = (temp64>>32) & 0xffffffff;
			lo = temp64 & 0xffffffff;

			post_log("Memory (data line) error at %08x, "
				  "wrote %08x%08x, read %08x%08x !\n",
					  pmem, pathi, patlo, hi, lo);
			ret = -1;
		}
	}
	return ret;
}


// 地址线测试	输入：测试地址，基地址，允许大小
// 基地址 和 允许大小 用于限制测试的内存地址范围
// 通过使用异或值，测试地址线的短路和断路。	
static int memory_post_addrline(ulong *testaddr, ulong *base, ulong size)
{
	// ulong 在32位系统中占32bit，64位系统中占64bit，与地址总线宽度一致。
	ulong *target;
	ulong *end;
	ulong readback;
	ulong xor;
	int   ret = 0;

	end = (ulong *)((ulong)base + size);	/* pointer arith! 指针运算！*/
	
	xor = 0;	// 异或值
	for(xor = sizeof(ulong); xor > 0; xor <<= 1) {	// xor 为 4 ，8，16，32 。二进制即 0 0 0 0100，0 0 0 1000
		target = (ulong *)((ulong)testaddr ^ xor);	// 指向地址为 testaddr 异或 xor, 即testaddr的某一位翻转
		
		// 限制！！ 地址不得超出基地址和结束地址
		// target 在 base 和 end 之间
		if((target >= base) && (target < end)) {	
			*testaddr = ~*target;	// target指向的地址的值取反，写入testaddr指向的地址
			readback  = *target;	// 读取target指向的地址的值

#ifdef INJECT_ADDRESS_ERRORS
			if(xor == 0x00008000) {	// 0x00008000 即 1000 0000 0000 0000
				readback = *testaddr;
			}
#endif
			if(readback == *testaddr) {	// 读回的值等于写入的值 报错
				post_log("Memory (address line) error at %08x<->%08x, "
					"XOR value %08x !\n",
					testaddr, target, xor);
				ret = -1;
			}
		}
	}
	return ret;
}



// 内存单元 1号测试
// 输入：start: ulong形式表示 起始地址
//		size: 待测试内存大小 单位：字节
static int memory_post_test1(unsigned long start,
			      unsigned long size,
			      unsigned long val)
{
	unsigned long i;
	ulong *mem = (ulong *) start;	// 指针 mem 对应 start 地址
	ulong readback;	
	int ret = 0;

	// 需测字节数 size
	// 一次测ulong大小的内存单元
	// 总测试步骤数即 size/sizeof(ulong)
	for (i = 0; i < size / sizeof (ulong); i++) {	
		mem[i] = val;	// mem为ulong指针，mem[i]即地址mem + i*sizeof(ulong)

		if (i % 1024 == 0)
			WATCHDOG_RESET();	// 64位：每测1024次，共计8KB,喂一次狗
	}

	for (i = 0; i < size / sizeof (ulong) && !ret; i++) {
		readback = mem[i];
		if (readback != val) {
			post_log("Memory error at %08x, "
				  "wrote %08x, read %08x !\n",
					  mem + i, val, readback);

			ret = -1;
			break;
		}
		if (i % 1024 == 0)
			WATCHDOG_RESET();
	}

	return ret;
}

static int memory_post_test2(unsigned long start, unsigned long size)
{
	unsigned long i;
	ulong *mem = (ulong *) start;
	ulong readback;
	int ret = 0;

	for (i = 0; i < size / sizeof (ulong); i++) {
		mem[i] = 1 << (i % 32);
		if (i % 1024 == 0)
			WATCHDOG_RESET();
	}

	for (i = 0; i < size / sizeof (ulong) && !ret; i++) {
		readback = mem[i];
		if (readback != (1 << (i % 32))) {
			post_log("Memory error at %08x, "
				  "wrote %08x, read %08x !\n",
					  mem + i, 1 << (i % 32), readback);

			ret = -1;
			break;
		}
		if (i % 1024 == 0)
			WATCHDOG_RESET();
	}

	return ret;
}

static int memory_post_test3(unsigned long start, unsigned long size)
{
	unsigned long i;
	ulong *mem = (ulong *) start;
	ulong readback;
	int ret = 0;

	for (i = 0; i < size / sizeof (ulong); i++) {
		mem[i] = i;
		if (i % 1024 == 0)
			WATCHDOG_RESET();
	}

	for (i = 0; i < size / sizeof (ulong) && !ret; i++) {
		readback = mem[i];
		if (readback != i) {
			post_log("Memory error at %08x, "
				  "wrote %08x, read %08x !\n",
					  mem + i, i, readback);

			ret = -1;
			break;
		}
		if (i % 1024 == 0)
			WATCHDOG_RESET();
	}

	return ret;
}

static int memory_post_test4(unsigned long start, unsigned long size)
{
	unsigned long i;
	ulong *mem = (ulong *) start;
	ulong readback;
	int ret = 0;

	for (i = 0; i < size / sizeof (ulong); i++) {
		mem[i] = ~i;
		if (i % 1024 == 0)
			WATCHDOG_RESET();
	}

	for (i = 0; i < size / sizeof (ulong) && !ret; i++) {
		readback = mem[i];
		if (readback != ~i) {
			post_log("Memory error at %08x, "
				  "wrote %08x, read %08x !\n",
					  mem + i, ~i, readback);

			ret = -1;
			break;
		}
		if (i % 1024 == 0)
			WATCHDOG_RESET();
	}

	return ret;
}

static int memory_post_test_lines(unsigned long start, unsigned long size)
{
	int ret = 0;

	ret = memory_post_dataline((unsigned long long *)start);
	WATCHDOG_RESET();
	if (!ret)
		ret = memory_post_addrline((ulong *)start, (ulong *)start,
				size);
	WATCHDOG_RESET();
	if (!ret)
		ret = memory_post_addrline((ulong *)(start+size-8),
				(ulong *)start, size);
	WATCHDOG_RESET();

	return ret;
}

static int memory_post_test_patterns(unsigned long start, unsigned long size)
{
	int ret = 0;

	ret = memory_post_test1(start, size, 0x00000000);
	WATCHDOG_RESET();
	if (!ret)
		ret = memory_post_test1(start, size, 0xffffffff);
	WATCHDOG_RESET();
	if (!ret)
		ret = memory_post_test1(start, size, 0x55555555);
	WATCHDOG_RESET();
	if (!ret)
		ret = memory_post_test1(start, size, 0xaaaaaaaa);
	WATCHDOG_RESET();
	if (!ret)
		ret = memory_post_test2(start, size);
	WATCHDOG_RESET();
	if (!ret)
		ret = memory_post_test3(start, size);
	WATCHDOG_RESET();
	if (!ret)
		ret = memory_post_test4(start, size);
	WATCHDOG_RESET();

	return ret;
}

static int memory_post_test_regions(unsigned long start, unsigned long size)
{
	unsigned long i;
	int ret = 0;

	for (i = 0; i < (size >> 20) && (!ret); i++) {
		if (!ret)
			ret = memory_post_test_patterns(start + (i << 20),
				0x800);
		if (!ret)
			ret = memory_post_test_patterns(start + (i << 20) +
				0xff800, 0x800);
	}

	return ret;
}

static int memory_post_tests(unsigned long start, unsigned long size)
{
	int ret = 0;

	ret = memory_post_test_lines(start, size);
	if (!ret)
		ret = memory_post_test_patterns(start, size);

	return ret;
}

/*
 * !! this is only valid, if you have contiguous memory banks !!
 */
__attribute__((weak))
int arch_memory_test_prepare(u32 *vstart, u32 *size, phys_addr_t *phys_offset)
{
	bd_t *bd = gd->bd;

	*vstart = CONFIG_SYS_SDRAM_BASE;
	*size = (gd->ram_size >= 256 << 20 ?
			256 << 20 : gd->ram_size) - (1 << 20);

	/* Limit area to be tested with the board info struct */
	if ((*vstart) + (*size) > (ulong)bd)
		*size = (ulong)bd - *vstart;

	return 0;
}

__attribute__((weak))
int arch_memory_test_advance(u32 *vstart, u32 *size, phys_addr_t *phys_offset)
{
	return 1;
}

__attribute__((weak))
int arch_memory_test_cleanup(u32 *vstart, u32 *size, phys_addr_t *phys_offset)
{
	return 0;
}

__attribute__((weak))
void arch_memory_failure_handle(void)
{
	return;
}

int memory_regions_post_test(int flags)
{
	int ret = 0;
	phys_addr_t phys_offset = 0;
	u32 memsize, vstart;

	arch_memory_test_prepare(&vstart, &memsize, &phys_offset);

	ret = memory_post_test_lines(vstart, memsize);
	if (!ret)
		ret = memory_post_test_regions(vstart, memsize);

	return ret;
}

int memory_post_test(int flags)
{
	int ret = 0;
	phys_addr_t phys_offset = 0;
	u32 memsize, vstart;

	arch_memory_test_prepare(&vstart, &memsize, &phys_offset);

	do {
		if (flags & POST_SLOWTEST) {
			ret = memory_post_tests(vstart, memsize);
		} else {			/* POST_NORMAL */
			ret = memory_post_test_regions(vstart, memsize);
		}
	} while (!ret &&
		!arch_memory_test_advance(&vstart, &memsize, &phys_offset));

	arch_memory_test_cleanup(&vstart, &memsize, &phys_offset);
	if (ret)
		arch_memory_failure_handle();

	return ret;
}

#endif /* CONFIG_POST&(CONFIG_SYS_POST_MEMORY|CONFIG_SYS_POST_MEM_REGIONS) */
