#include "dev/time.h"
#include "comm/types.h"
#include "comm/cpu_instr.h"
#include "os_cfg.h"
#include "cpu/irq.h"
#include "core/task.h"

static uint32_t sys_tick;

void do_handle_time(exception_frame_t * frame)
 {
    sys_tick++;
    // 先发EOI，而不是放在最后
    // 放最后将从任务中切换出去之后，除非任务再切换回来才能继续
    pic_send_eoi(IRQ0_TIMER);
    
    task_time_tick();
 }

static void init_pit(void)
{
    //PIT_OSC_FREQ 意思是1秒钟有PIT_OSC_FREQ个时钟节拍进来
    //每一个时钟节拍的时长是 1000/PIT_OSC_FREQ
    uint32_t reload_count = PIT_OSC_FREQ * OS_TICKS_MS / 1000;
    outb(PIT_COMMAND_MODE_PORT, PIT_CHANNLE | PIT_LOAD_LOHI | PIT_MODE3);
    outb(PIT_CHANNEL0_DATA_PORT, reload_count & 0xFF);        // 加载低8位
    outb(PIT_CHANNEL0_DATA_PORT, (reload_count >> 8) & 0xFF); // 再加载高8位

    irq_install(IRQ0_TIMER, (irq_handle_t)exception_handler_time);
    irq_enable(IRQ0_TIMER);
}


void time_init (void)
{
    sys_tick = 0;
    init_pit();
}