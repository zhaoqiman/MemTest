#include "mem_test.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/time.h>


#define TEST_SIZE 100
#define LOOP_TIME 1000

extern ErrorInfo error_info;

int main(){
    ulong arr[TEST_SIZE] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    ulong* test_addr = &arr[0];
    int test_size = TEST_SIZE * sizeof(ulong);

    int res = 1;
    struct timeval start,end;

    // Checkerboard test Time Count
    gettimeofday(&start, NULL );
    for (size_t i = 0; i < LOOP_TIME; i++)
    {
        res = CheckerboardTest(test_addr, test_size);
    }
    gettimeofday(&end, NULL );
    long time_use =1000000 * ( end.tv_sec - start.tv_sec ) + end.tv_usec - start.tv_usec;  
    printf("CheckerboardTest time: %ld us\n", time_use);

    if(res == 1){
        printf("Checkerboard test passed.\n");
    }
    else{
        printf("Checkerboard test failed.\n");
        printf("Error at Address 0x%lX， ulong write value 0x%lX, ulong read value  0x%lX\n", error_info.addr, error_info.write_val, error_info.read_val);
    }


    // Walking test Time Count
    gettimeofday(&start, NULL );
    for (size_t i = 0; i < LOOP_TIME; i++)
    {
        res = WalkingTest(test_addr, test_size);
    }
    gettimeofday(&end, NULL );
    time_use =1000000 * ( end.tv_sec - start.tv_sec ) + end.tv_usec - start.tv_usec;
    printf("WalkingTest time: %ld us\n", time_use);
    if(res == 1){
        printf("Walking test passed.\n");
    }
    else{
        printf("Walking test failed.\n");
        printf("Error at Address 0x%lX， ulong write value 0x%lX, ulong read value  0x%lX\n", error_info.addr, error_info.write_val, error_info.read_val);
    }


    // MarchCTest test Time Count
    gettimeofday(&start, NULL );
    for (size_t i = 0; i < LOOP_TIME; i++)
    {
        res = MarchCTest(test_addr, test_size);
    }
    gettimeofday(&end, NULL );
    time_use =1000000 * ( end.tv_sec - start.tv_sec ) + end.tv_usec - start.tv_usec;
    printf("MarchCTest time: %ld us\n", time_use);
    if(res == 1){
        printf("MarchC test passed.\n");
    }
    else{
        printf("MarchC test failed.\n");
        printf("Error at Address 0x%lX， ulong write value 0x%lX, ulong read value  0x%lX\n", error_info.addr, error_info.write_val, error_info.read_val);
    }


    return 0;

}