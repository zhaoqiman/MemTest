#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <ctime>
#include <random>
#include <cstdlib>

#define DEFAULT_WIDTH 32
#define DEFAULT_TESTTIME 10000
//数据长 /字节
int WIDTH = DEFAULT_WIDTH;
// 总测试次数
unsigned int TESTTIME = DEFAULT_TESTTIME;   

int crc_init = 0;
unsigned int crc_table[256];
std::mutex mtx;

unsigned long collisionCount= 0;
unsigned long crcCount= 0;

// 初始化 CRC32 校验表
void crc32gentab(void) {
    unsigned long crc, poly;
    int i, j;

    poly = 0xEDB88320L;
    for (i = 0; i < 256; i++) {
        crc = i;
        for (j = 8; j > 0; j--) {
            if (crc & 1) {
                crc = (crc >> 1) ^ poly;
            }
            else {
                crc >>= 1;
            }
        }
        crc_table[i] = crc;
    }
}

//  计算校验值
unsigned int crc32(unsigned char* buf, unsigned int len) {
    unsigned int crc;

    if (crc_init == 0) {
        crc32gentab();
        crc_init = 1;
    }

    crc = 0xffffffff;
    while (len--) {
        crc = crc_table[(crc ^ (unsigned char)*buf++) & 0xff] ^ (crc >> 8);
    }
    return crc ^ 0xffffffff;
}

//  计算汉明距离
int hammingDist(unsigned int a, unsigned int b) {
    int distance = 0;
    unsigned int xor_result = a ^ b;
    while (xor_result) {
        distance += xor_result & 1;
        xor_result >>= 1;
    }
    return distance;
}



// 线程执行totalTime次测试函数
void oneTest(unsigned long& collisionCount, unsigned long& crcCount, int id, int totalTime) {
    srand((unsigned)time(0) + id<<8);
    unsigned char x[WIDTH];
    unsigned char y[WIDTH];

    int i = 0, j = 0;

    while (totalTime > 0) {
        // x 随机
        for (i = 0; i != WIDTH; i++) {
            x[i] = rand() % 255 + 1;
            y[i] = x[i];
        }

        unsigned int Cx = crc32(&x[0], WIDTH);
        unsigned int Cy = 0;

        int dist = 0;
        // 原 x 逐 bit翻转，即错码 y
        for (i = 0; i != WIDTH; i++) {      // each Byte of x
            for (j = 0; j != 8; j++) {      // each bit of each Byte
                y[i] ^= 1 << j; // 第i字节 第j比特 反转
                Cy = crc32(&y[0], WIDTH);
                dist = hammingDist(Cx, Cy);

                switch (dist) {
                case 0:
                    mtx.lock();
                    collisionCount++;
                    mtx.unlock();
                    break;
                case 1:
                    mtx.lock();
                    crcCount++;
                    mtx.unlock();
                    break;
                default:
                    break;
                }
                //再反转 恢复 y 原样
                y[i] ^= 1 << j;
            }
        }
        totalTime--;
    }
}

int main(int argc, char* argv[]) {

    // 检查命令行参数数量
    if (argc >= 3) {
        WIDTH = std::atoi(argv[1]); // 第一个参数是 WIDTH
        TESTTIME = std::atoi(argv[2]); // 第二个参数是 TESTTIME
    }

    int numThreads = std::thread::hardware_concurrency(); // 获取可用的线程数量

    unsigned int totalTime = TESTTIME;
    //unsigned long collisionCount = 0, crcCount = 0;

    //  创建numThreads个线程向量，每个线程执行 totalTime / numThreads 次
    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; ++i) {
        threads.push_back(std::thread(oneTest, std::ref(collisionCount), std::ref(crcCount), i, totalTime / numThreads));
    }

    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }

    unsigned long long allNum = totalTime * WIDTH * 8;
    std::cout << "Total tested code num is " << totalTime << std::endl;
    std::cout << "Total possible wrong code num is " << allNum << std::endl;
    std::cout << "In which, collisionCount is " << collisionCount << std::endl;
    std::cout << "crcCount is " << crcCount << std::endl;

    return 0;
}
