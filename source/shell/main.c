#include "lib_syscall.h"
#include <stdio.h>

int main(int argc, char ** argv)
{
    for (int i = 0; i < argc; i++)
    {
        printf("arg: %s\n", argv[i]);
    }
    sbrk(0);
    sbrk(100);
    sbrk(200);
    sbrk(4096 * 2 + 200);

    // \b光标左移
    printf("abcd\b\b\b\bcd\n");
    //\x7f向左删除字符
    printf("abcd\x7f;fg\n");

    printf("hello shell\n");
    for (int i = 0; i < argc; i++)
    {
        printf("arg: %s\n", argv[i]);
    }
    fork();
    for(;;)
    {
        //printf("task id:%d\n", gettid());
        yeild();
        sleep(1000);
    }
}