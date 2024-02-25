#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#include "comm/types.h"

#define SYS_SLEEP                   0
#define SYS_GETTID                  1
#define SYS_FORK                    3
#define SYS_PRINT_MSG               100


#define SYSCALL_PARAM_COUNT         5

typedef struct _syscall_frame_t
{
    int eflags;
    int gs, fs, es, ds;
    uint32_t edi, esi, ebp, dummy, ebx, edx, ecx, eax;
    int eip, cs;
    int func_id, arg0, arg1, arg2, arg3;
    int esp, ss;
} syscall_frame_t;

void exception_handle_syscall(void);

#endif