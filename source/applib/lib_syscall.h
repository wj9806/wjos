#ifndef __LIB_SYSCALL_H__
#define __LIB_SYSCALL_H__

#include "os_cfg.h"
#include "core/syscall.h"

typedef struct _syscall_args_t
{
    //操作系统函数id
    int id;
    //参数
    int arg0;
    int arg1;
    int arg2;
    int arg3;
} syscall_args_t;

static inline int sys_call(syscall_args_t* args)
{
    int ret;
    uint32_t addr[] = {0, SELECTOR_SYSCALL | 0};
    __asm__ __volatile__(
        "push %[arg3]\n\t"
        "push %[arg2]\n\t"
        "push %[arg1]\n\t"
        "push %[arg0]\n\t"
        "push %[id]\n\t"
        "lcalll *(%[a])"
        :"=a"(ret)
        :[arg3]"r"(args->arg3), [arg2]"r"(args->arg2),
        [arg1]"r"(args->arg1), [arg0]"r"(args->arg0), [id]"r"(args->id),
        [a]"r"(addr));

    return ret;
}


static inline void sleep(int ms)
{
    if (ms <= 0)
    {
        return;
    }

    syscall_args_t args;
    args.id = SYS_SLEEP;
    args.arg0 = ms;

    sys_call(&args);
}

static inline int gettid(void)
{
    syscall_args_t args;
    args.id = SYS_GETTID;

    return sys_call(&args);
}

static inline void print_msg(const char * fmt, int arg)
{
    syscall_args_t args;
    args.id = SYS_PRINT_MSG;
    args.arg0 = (int)fmt;
    args.arg1 = arg;

    sys_call(&args);
}

static inline int fork()
{
    syscall_args_t args;
    args.id = SYS_FORK;
    sys_call(&args);
}

#endif