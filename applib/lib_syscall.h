#ifndef __LIB_SYSCALL_H__
#define __LIB_SYSCALL_H__

#include <sys/stat.h>
#include <stddef.h>

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

struct dirent
{
    int index;
    int type;
    char name[255];
    int size;
};


typedef struct _DIR
{
    int index;
    struct dirent dirent;
    
} DIR;


void sleep(int ms);

int getpid(void);

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

int isatty(int file);

int fstat(int file, struct stat * st);

//用于增长应用程序的数据空间increment字节
//当incr=0时，返回当前的program break
//成功时，返回新分配空间的起始地址
//错误返回-1
void * sbrk(ptrdiff_t incr);

int dup(int file);

void _exit(int status);

int wait(int * status);

DIR * opendir(const char * path);

struct dirent * readdir(DIR * dir);

int closedir(DIR * dir);

#endif