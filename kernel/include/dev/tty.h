#ifndef __TTY_H__
#define __TTY_H__

#include "ipc/sem.h"

#define TTY_NR          8
#define TTY_OBUF_SIZE   512
#define TTY_IBUF_SIZE   512
#define TTY_OCRLF       (1<<0)
#define TTY_INCLR       (1<<0)
#define TTY_IECHO       (1<<1)

/**
 * buf
 * --------------------------------------------------------------
 * |    |   |   |   |   |   |   |   |   |   |   |   |   |   |   |     
 * |    |   |   |   |   |   |   |   |   |   |   |   |   |   |   |        
 * --------------------------------------------------------------
 *               /\  <-------有效数据------>  /\ 
 *               ||                          ||
 *              read                        write
 * 
 * 进程写入缓存，然后启动硬件发送机制
*/

//先进先出 循环队列
typedef struct _tty_fifo_t
{
    char * buf;
    int size;
    int read, write;
    int count;
} tty_fifo_t;


typedef struct _tty_t
{
    char obuf[TTY_OBUF_SIZE];
    char ibuf[TTY_IBUF_SIZE];
    //输出缓存
    tty_fifo_t ofifo;
    //输入缓存
    tty_fifo_t ififo;

    sem_t osem;
    sem_t isem;

    int oflags;
    int iflags;

    int console_idx;
} tty_t;

int tty_fifo_put(tty_fifo_t * fifo, char c);

int tty_fifo_get(tty_fifo_t * fifo, char *c);

void tty_in(int idx, char ch);

#endif