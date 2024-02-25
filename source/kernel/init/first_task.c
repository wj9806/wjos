#include "applib/lib_syscall.h"

int first_task_main(void)
{
    int tid = gettid();
    print_msg("task id=%d", tid);

    //创建子进程
    int pid = fork();
    if (pid < 0)
    {
        print_msg("create child proc failed：%d", tid);
    } else if (pid == 0)
    {
        print_msg("child: %d", pid);
    } else {
        print_msg("child task id: %d", pid);
        print_msg("parent: %d", tid);
    }

    for(;;)
    {
        
        sleep(1000);
    }

    return 0;
}