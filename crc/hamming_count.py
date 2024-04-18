"""
    zhaoqiman
    统计不同错码位数下，CRC校验码的汉明距离
    注意:   统计所有误码情况，复杂度为 Combination(data_len*8, error_bits)。
            内存占用、运行时长可能会非常高，请谨慎设置参数。
    TODO:   性能优化。多线程计算。
"""

import itertools
import numpy as np
import os
import matplotlib.pyplot as plt
import crcmod

''' Global Variables
data_len: data length in bytes
crc_len: crc length in bytes
error_bits: error bit num of data
test_time: total random test times
crc_algorithm: 'crc-32', 'crc-16'... 
'''
# User defined variables
data_len = 4
crc_len = 4
error_bits = 4
test_time = 10
crc_algorithm = 'crc-32'
# Calculated variables
bit_num = data_len * 8
crc_func = crcmod.predefined.Crc(crc_algorithm)


def calc_crc(data):
    """
        Modify 'crc_algorithm' if you want to use other crc algorithm
        ps: Do not change the function name
    """
    return crc_func.new(data).crcValue


def hamming_distance(s1, s2):
    """
        Return the hamming distance between s1 and s2
    """
    return bin(s1 ^ s2).count('1')


def show_dist_diagram(dist_list):
    """
        Show the distribution diagram of hamming distance
        dist_list: list of hamming distance
    """
    x = np.arange(len(dist_list))
    y = np.array(dist_list)
    total = np.sum(y)
    plt.bar(x, y / total * 100, width=0.5, align='center')
    plt.xlabel('Hamming Distance')
    plt.ylabel('Frequency/%')
    plt.title('Hamming Distance Distribution. Error Bits: %d' % error_bits)
    plt.show()


def random_test(test_time, error_bits, bit_num, dist_list):
    """  Generate random data and test.
    Args:
        error_bits: error bit num of data
        bit_num: total bit num of data
        dist_list: list of hamming distance to be saved
    """
    # Generate all possible error patterns
    # error_bits in total bit_num bits
    error_patterns = []
    error_patterns.extend(list(itertools.combinations(range(bit_num), error_bits)))

    # generate data and calculate hamming distance For test_time times
    for i in range(test_time):
        # random data and calculate original crc
        data_original = os.urandom(bit_num)
        data_original = bytearray(data_original)
        crc_original = calc_crc(data_original)
        # loop all error patterns
        for pattern in error_patterns:
            data_modified = bytearray(data_original)
            # flip bits according to error pattern
            for bit_index in pattern:
                data_modified[bit_index // 8] ^= (1 << (bit_index % 8))
            crc_modified = calc_crc(data_modified)
            hamming_dist = hamming_distance(crc_original, crc_modified)
            dist_list[hamming_dist] += 1
        # Print dist_list for every random data If needed
        # print(dist_list)


if __name__ == "__main__":
    # 保存所有测试的汉明距离
    temp_dist_list = [0] * (crc_len * 8 + 1)

    # 指定 错误位数 error_bits 测试
    random_test(test_time, error_bits, bit_num, temp_dist_list)
    print(temp_dist_list)
    show_dist_diagram(temp_dist_list)

    '''
    # 错误位数从 1 到 error_bits 测试
    for possible_error_bits in range(1, error_bits + 1):
        temp_dist_list = [0] * (crc_len * 8 + 1)
        random_test(test_time, possible_error_bits, bit_num, temp_dist_list)
        print('Error Bits: %d. Total test errors: %d.' % (possible_error_bits, np.sum(temp_dist_list)))
        print('Minimum Hamming Distance :', np.nonzero(temp_dist_list)[0][0])
        print(temp_dist_list)
    '''
