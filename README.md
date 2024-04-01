# RAM_Test

#### 介绍
毕业设计相关。
本仓库存放RAM测试算法实现相关代码。

#### u-boot下使用mtest工具
编译u-boot前需在config文件中添加

    CONFIG_CMD_MEMTEST=y

使用方法为

    mtest [start [end [pattern [iterations]]]]

示例

    U-Boot> mtest 0x20000000 0x22000000 0xaabbccdd
    Testing 20000000 ... 22000000:
    Pattern FFFFFFFF55443322  Writing...  Reading...Iteration:   1094
    Tested 1094 iteration(s) with 0 errors.

注意：测试地址不得覆盖u-boot所在内存


#### 所用软硬件
raspberrypi内核编译openEuler
openEuler 22.03 LTS：git clone git@gitee.com:openeuler/raspberrypi-kernel.git -b openEuler-22.03-LTS && cd raspberrypi-kernel

#### 链接跳转
树莓派 openeuler 编译:
    https://gitee.com/openeuler/raspberrypi/blob/master/documents/openEuler%E9%95%9C%E5%83%8F%E7%9A%84%E6%9E%84%E5%BB%BA.md#%E4%B8%8B%E8%BD%BD%E6%BA%90%E7%A0%81

x86环境交叉编译aarch64的openeuler 

https://gitee.com/openeuler/raspberrypi/blob/master/documents/%E4%BA%A4%E5%8F%89%E7%BC%96%E8%AF%91%E5%86%85%E6%A0%B8.md#%E4%B8%8B%E8%BD%BD%E5%86%85%E6%A0%B8%E6%BA%90%E7%A0%81