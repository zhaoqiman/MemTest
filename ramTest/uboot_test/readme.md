## u-boot 自定义测试指令 march, walking, checker
在u-boot环境下，添加自定义指令，
用于实现 March C- 算法测试、Walking 算法测试、Checkerboard 算法测试。
指定起始地址和内存大小，对指定内存区域进行测试

用法： 
```shell
march [start_addr] [test_size]
walking [start_addr] [test_size]
checker [start_addr] [test_size]
```
start_addr - 测试起始地址，raspberry pi 4b 中应大于 0x80000

test_size - 测试大小(字节)

示例： 
```shell
march 00ff0000 1024
```

### 如需使用
将cmd_march.c , cmd_walking.c, cmd_checker.c 三个文件放入 u-boot\common\ 目录下.

修改u-boot源码根目录下的 u-boot\common\Makefile 文件，添加以下内容：
```shell
obj-y += cmd_march.o
obj-y += cmd_walking.o
obj-y += cmd_checker.o
```
重新编译 u-boot
```
# 将aarch64-rpi4-linux-gnu交叉编译器的路径作为环境变量
$ export PATH=${HOME}/x-tools/aarch64-rpi4-linux-gnu/bin/:$PATH
# 选择树莓派4的参数做配置
$ make CROSS_COMPILE=aarch64-rpi4-linux-gnu- rpi_4_defconfig
# 编译生成可执行文件
$ make CROSS_COMPILE=aarch64-rpi4-linux-gnu- -j4
```