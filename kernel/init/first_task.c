#include "applib/lib_syscall.h"
#include "dev/tty.h"

int first_task_main(void)
{
    print_msg("welcome to wjos", 0);
    
    int count = 0;
#if 0
    int tid = getpid();
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
#endif
    for (int i = 0; i < 1; i++)
    {
        int pid = fork();
        if (pid < 0)
        {
            print_msg("create shell failed", 0);
            break;
        }
        else if (pid == 0)
        {   
            char tty_num[] = "/dev/tty?";
            tty_num[sizeof(tty_num) - 2] = i + '0';
            char* argv[] = {tty_num, (char*)0};
            execve("/shell.elf", argv, (char**)0);
        }
    }


    for(;;)
    {
        //sleep(1000);
        
        int status;
        wait(&status);
    }

    return 0;
}