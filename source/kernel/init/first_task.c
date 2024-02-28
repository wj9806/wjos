#include "applib/lib_syscall.h"

int first_task_main(void)
{
    int count = 0;

    int tid = gettid();
    print_msg("task id=%d", tid);

    //创建子进程
    //fork后子进程和父进程不共用同一块数据
    tid = fork();
    print_msg("fork result:%d", tid);

    if (tid < 0)
    {
        print_msg("create child proc failed：%d", tid);
    } else if (tid == 0)
    {
        count = 100;
        print_msg("child: %d", tid);
        char* argv[] = {"arg1", "arg2"};
        execve("/shell.elf", argv, (char**)0);
    } else {
        print_msg("child task id: %d", tid);
        print_msg("parent: %d", tid);
    }

    for(;;)
    {
        sleep(1000);
        //print_msg("for task id=%d", tid);
        //print_msg("count=%d", count);
    }

    return 0;
}