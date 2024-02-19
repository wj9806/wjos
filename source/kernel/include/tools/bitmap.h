#ifndef __BITMAP_H__
#define __BITMAP_H__

#include "comm/types.h"

typedef struct _bitmap_t {
    //位的个数
    int bit_count;
    //字节数组
    uint8_t * bits;
} bitmap_t;

int bitmap_byte_count(int bit_count);

void bitmap_init(bitmap_t * bitmap, uint8_t * bits, int count, int init_bit);

int bitmap_get_bit(bitmap_t * bitmap, int index);

//从index开始，连续count位置，设置成bit
void bitmap_set_bit(bitmap_t * bitmap, int index, int count, int bit);

//判断index位置是0是1
int bitmap_is_set(bitmap_t * bitmap, int index);

//count连续为bit的位置进行分配
int bitmap_alloc_nbits(bitmap_t * bitmap, int bit, int count);

#endif