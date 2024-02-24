#include "applib/lib_syscall.h"

int first_task_main(void)
{
    int tid = gettid();
    for(;;)
    {
        print_msg("task id=%d", tid);
        sleep(1000);
    }

    return 0;
}