#include "applib/lib_syscall.h"

int first_task_main(void)
{
    for(;;)
    {
        sleep(1000);
    }

    return 0;
}