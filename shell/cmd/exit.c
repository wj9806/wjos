#include "cmd/cmd.h"
#include "applib/lib_syscall.h"
#include <stdlib.h>

int do_exit(int argc, char ** argv)
{
    exit(0);
    return 0;
}