#ifndef __TIMER_H__
#define __TIMER_H__

// 定时器的寄存器和各项位配置
#define PIT_OSC_FREQ             1193182
#define PIT_COMMAND_MODE_PORT    0x43
#define PIT_CHANNEL0_DATA_PORT   0x40

#define PIT_CHANNLE                 (0 << 6)
#define PIT_LOAD_LOHI               (3 << 4)
#define PIT_MODE3                   (3 << 1)

void time_init (void);

void exception_handler_time (void);

#endif