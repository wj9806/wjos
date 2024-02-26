#include "lib_syscall.h"

int main(int argc, char ** argv);

//调用前的初始化
void cstart(int argc, char ** argv)
{
    for (int i = 0; i < argc; i++)
    {
        print_msg("arg: %s", (int)argv[i]);
    }
    
    main(argc, argv);
}