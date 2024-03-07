#include "applib/lib_syscall.h"
#include "dev/tty.h"

int first_task_main(void)
{
    print_msg("welcome to wjos", 0);
    
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
            execve("shell.elf", argv, (char**)0);
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