#include "lib_syscall.h"
#include "os_cfg.h"
#include "core/syscall.h"
#include <stdlib.h>

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

void sleep(int ms)
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

int getpid(void)
{
    syscall_args_t args;
    args.id = SYS_GETPID;

    return sys_call(&args);
}

void print_msg(const char * fmt, int arg)
{
    syscall_args_t args;
    args.id = SYS_PRINT_MSG;
    args.arg0 = (int)fmt;
    args.arg1 = arg;

    sys_call(&args);
}

int fork()
{
    syscall_args_t args;
    args.id = SYS_FORK;
    sys_call(&args);
}

//在父进程中fork一个子进程，在子进程中调用exec函数启动新的程序
int execve(const char * name, char *const argv[], char *const envp[])
{
    syscall_args_t args;
    args.id = SYS_EXECVE;
    args.arg0 = (int)name;
    args.arg1 = (int)argv;
    args.arg2 = (int)envp;
    return sys_call(&args);
}

int yeild()
{
    syscall_args_t args;
    args.id = SYS_YEILD;
    sys_call(&args);
}

int open(const char * name, int flags, ...)
{
    syscall_args_t args;
    args.id = SYS_OPEN;
    args.arg0 = (int)name;
    args.arg1 = flags;

    return sys_call(&args);
}

int read(int file, char * ptr, int len)
{
    syscall_args_t args;
    args.id = SYS_READ;
    args.arg0 = file;
    args.arg1 = (int)ptr;
    args.arg2 = len;

    return sys_call(&args);
}

int write(int file, char * ptr, int len)
{
    syscall_args_t args;
    args.id = SYS_WRITE;
    args.arg0 = file;
    args.arg1 = (int)ptr;
    args.arg2 = len;

    return sys_call(&args);
}

int close(int file)
{
    syscall_args_t args;
    args.id = SYS_CLOSE;
    args.arg0 = file;

    return sys_call(&args);
}

int lseek(int file, int ptr, int dir)
{
    syscall_args_t args;
    args.id = SYS_LSEEK;
    args.arg0 = file;
    args.arg1 = (int)ptr;
    args.arg2 = dir;

    return sys_call(&args);
}

int isatty(int file)
{
    syscall_args_t args;
    args.id = SYS_ISATTY;
    args.arg0 = file;

    return sys_call(&args);
}

int ioctl(int file, int cmd, int arg0, int arg1)
{
    syscall_args_t args;
    args.id = SYS_IOCTL;
    args.arg0 = file;
    args.arg1 = cmd;
    args.arg2 = arg0;
    args.arg3 = arg1;

    return sys_call(&args);
}

int fstat(int file, struct stat * st)
{
    syscall_args_t args;
    args.id = SYS_FSTAT;
    args.arg0 = file;
    args.arg1 = (int)st;

    return sys_call(&args);
}

void * sbrk(ptrdiff_t incr)
{
    syscall_args_t args;
    args.id = SYS_SBRK;
    args.arg0 = (int)incr;
    return (void *)sys_call(&args);
}

int dup(int file)
{
    syscall_args_t args;
    args.id = SYS_DUP;
    args.arg0 = file;
    return sys_call(&args);
}

void _exit(int status)
{
    syscall_args_t args;
    args.id = SYS_EXIT;
    args.arg0 = status;
    sys_call(&args);

    //warning: ‘noreturn’ function does return
    for(;;){}
}

int wait(int * status)
{
    syscall_args_t args;
    args.id = SYS_WAIT;
    args.arg0 = (int) status;
    return sys_call(&args);
}

DIR * opendir(const char * path)
{
    DIR * dir = (DIR *) malloc(sizeof(DIR)); 
    if (dir == (DIR *)0)
    {
        return dir;
    }
    syscall_args_t args;
    args.id = SYS_OPENDIR;
    args.arg0 = (int) path;
    args.arg1 = (int) dir;
    int err = sys_call(&args);
    if (err < 0)
    {
        free(dir);
        return (DIR *)0;
    }
    return dir;
}

struct dirent * readdir(DIR * dir)
{
    syscall_args_t args;
    args.id = SYS_READDIR;
    args.arg0 = (int) dir;
    args.arg1 = (int) &dir->dirent;
    int err = sys_call(&args);
    if (err < 0)
    {
        return (struct dirent *)0;
    }
    return &dir->dirent;
}

int closedir(DIR * dir)
{
    syscall_args_t args;
    args.id = SYS_CLOSEDIR;
    args.arg0 = (int) dir;
    sys_call(&args);
    free(dir);
    return 0;
}

int unlink(const char * pathname)
{
    syscall_args_t args;
    args.id = SYS_UNLINK;
    args.arg0 = (int) pathname;
    
    return sys_call(&args);
}

int gettimeofday(struct timeval* tv, timezone * tz)
{
    syscall_args_t args;
    args.id = SYS_GETTIMEOFDAY;
    args.arg0 = (int) tv;
    args.arg1 = (int) tz;
    
    return sys_call(&args);
}

struct tm * gmtime_r(const time_t *timep, struct tm * result)
{
    syscall_args_t args;
    args.id = SYS_GMTIME_R;
    args.arg0 = (int) timep;
    args.arg1 = (int) result;
    
    return (struct tm *)sys_call(&args);
}