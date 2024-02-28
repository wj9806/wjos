#include<stdarg.h>
#include "tools/log.h"
#include "tools/klib.h"
#include "comm/cpu_instr.h"
#include "ipc/mutex.h"
#include "dev/console.h"

#define LOG_USE_COM         0
#define COM1_PORT           0x3F8       // RS232端口0初始化

static mutex_t mutex;

void log_init(void)
{
    mutex_init(&mutex);
#if LOG_USE_COM
    outb(COM1_PORT + 1, 0x00);    // Disable all interrupts
    outb(COM1_PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    outb(COM1_PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
    outb(COM1_PORT + 1, 0x00);    //                  (hi byte)
    outb(COM1_PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
    outb(COM1_PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
  
    // If serial is not faulty set it in normal operation mode
    // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
    outb(COM1_PORT + 4, 0x0F);
#endif
}

void log_printf(const char * fmt, ...)
{
    char str_buf[128];
    va_list args;
    //格式化
    kernel_memset(str_buf, 0, 128);
    va_start(args, fmt);
    kernel_vsprintf(str_buf, fmt, args);
    va_end(args);

    const char* p = str_buf;

    mutex_lock(&mutex);
#if LOG_USE_COM
    while (*p != '\0')
    {
        while ((inb(COM1_PORT+5) & (1 << 6)) == 0);
        
        outb(COM1_PORT, *p++);
    }
    outb(COM1_PORT, '\r');
    outb(COM1_PORT, '\n');
#else
    console_write(0, str_buf, kernel_strlen(str_buf));
    char c = '\n';
    console_write(0, &c, 1);
#endif
    mutex_unlock(&mutex);
}