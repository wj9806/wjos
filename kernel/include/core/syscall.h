#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#include "comm/types.h"

#define SYS_SLEEP                   0
#define SYS_GETPID                  1
#define SYS_EXECVE                  2
#define SYS_FORK                    3
#define SYS_YEILD                   4
#define SYS_EXIT                    5
#define SYS_WAIT                    6

#define SYS_OPEN                    50
#define SYS_READ                    51
#define SYS_WRITE                   52
#define SYS_CLOSE                   53
#define SYS_LSEEK                   54

#define SYS_ISATTY                  55
#define SYS_FSTAT                   56
#define SYS_SBRK                    57
#define SYS_DUP                     58
#define SYS_IOCTL                   59

#define SYS_OPENDIR                 60
#define SYS_READDIR                 61
#define SYS_CLOSEDIR                62
#define SYS_UNLINK                  63

#define SYS_PRINT_MSG               100

#define SYS_GETTIMEOFDAY            120
#define SYS_GMTIME_R                121

#define SYS_POWER                   200
#define SYS_SAVE_HISTORY            201

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