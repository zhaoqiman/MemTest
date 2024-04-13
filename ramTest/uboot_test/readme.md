## u-boot 自定义测试指令 march
在u-boot环境下，添加一个自定义指令，
用于实现 March C- 测试算法
指定起始地址和测试大小，对指定内存区域进行 march C -算法测试

用法： 
```shell
march [start_addr] [test_size]
```
start_addr - 测试起始地址，raspberry pi 4b 中应大于 0x80000

test_size - 测试大小(字节)

示例： 
```shell
march 00ff0000 1024
```

### 如需使用
将cmd_march.c'文件放入 u-boot\common\ 目录下.

修改u-boot源码根目录下的 u-boot\common\Makefile 文件，添加以下内容：
```shell
obj-y += cmd_march.o
```
重新编译 u-boot
