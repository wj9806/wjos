#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#define KBD_PORT_DATA           0x60
#define KBD_PORT_STAT           0X64
#define KBD_PORT_CMD            0X64

#define KBD_STAT_RECV_READY     (1 << 0)

void keyboard_init (void);

void exception_handler_keyboard (void);


#endif