#include "dev/keyboard.h"
#include "cpu/irq.h"
#include "tools/log.h"
#include "comm/cpu_instr.h"

void keyboard_init (void)
{
    irq_install(IRQ1_KEYBOARD, (irq_handle_t) exception_handler_keyboard);
    irq_enable(IRQ1_KEYBOARD);
}

void do_handle_keyboard (exception_frame_t * frame)
{
    uint32_t status = inb(KBD_PORT_STAT);
    if (!(status & KBD_STAT_RECV_READY))
    {
        pic_send_eoi(IRQ1_KEYBOARD);
        return;
    }

    uint8_t raw_code = inb(KBD_PORT_DATA);
    log_printf("key: %d", raw_code);
    pic_send_eoi(IRQ1_KEYBOARD);
}