#ifndef __LIB_SYSCALL_H__
#define __LIB_SYSCALL_H__

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

void sleep(int ms);

int gettid(void);

void print_msg(const char * fmt, int arg);

int fork();

//在父进程中fork一个子进程，在子进程中调用exec函数启动新的程序
int execve(const char * name, char *const argv[], char *const envp[]);

int yeild();

int open(const char * name, int flags, ...);

int read(int file, char * ptr, int len);

int write(int file, char * ptr, int len);

int close(int file);

int lseek(int file, int ptr, int dir);

#endif