#include "lib_syscall.h"
#include <stdio.h>

int main(int argc, char ** argv)
{
    printf("hello shell\n");
    for (int i = 0; i < argc; i++)
    {
        print_msg("arg: %s", (int)argv[i]);
    }
    fork();
    for(;;)
    {
        print_msg("task id:%d", gettid());
        yeild();
        sleep(1000);
    }
}