#include "tools/bitmap.h"
#include "tools/klib.h"

int bitmap_byte_count(int bit_count)
{
    return (bit_count + 8 -1) / 8;
}

void bitmap_init(bitmap_t * bitmap, uint8_t * bits, int count, int init_bit)
{
    bitmap->bit_count = count;
    bitmap->bits = bits;
    int bytes = bitmap_byte_count(bitmap->bit_count);
    //设置成1 0xFF
    kernel_memset(bitmap->bits, init_bit ? 0xFF: 0, bytes);
}

int bitmap_get_bit(bitmap_t * bitmap, int index)
{
    return bitmap->bits[index/8] & (1 << (index % 8));
}

//从index开始，连续count位置，设置成bit
void bitmap_set_bit(bitmap_t * bitmap, int index, int count, int bit)
{
    for (int i = 0; (i < count) && (index < bitmap->bit_count); i++, index++)
    {
        if (bit)
        {
            bitmap->bits[index/8] |= (1 << (index % 8));
        }
        else
        {
            bitmap->bits[index/8] &= ~(1 << (index % 8));
        }
        
    }
    
}

//判断index位置是0是1
int bitmap_is_set(bitmap_t * bitmap, int index)
{
    return bitmap_get_bit(bitmap, index) ? 1 : 0;
}

//count连续为bit的位置进行分配
int bitmap_alloc_nbits(bitmap_t * bitmap, int bit, int count)
{
    int search_index = 0;
    int ok_index = -1;

    while(search_index < bitmap->bit_count)
    {
        // 定位到第一个相同的索引处
        if (bitmap_get_bit(bitmap, search_index) != bit)
        {
            search_index++;
            continue;
        }
        ok_index = search_index;
        int i;
        for (i = 1; (i < count) && (search_index < bitmap->bit_count); i++)
        {
            if (bitmap_get_bit(bitmap, search_index++) != bit)
            {
                // 不足count个，退出，重新进行最外层的比较
                ok_index = -1;
                break;
            }
        }

        //找到
        if (i >= count)
        {
            bitmap_set_bit(bitmap, ok_index, count, ~bit);
            return ok_index;
        }

    }
    return -1;
}