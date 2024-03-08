#include "cmd/cmd.h"
#include "applib/lib_syscall.h"

int do_rm(int argc, char ** argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "no file");
        return -1;
    }
    int err = unlink(argv[1]);
    if (err < 0)
    {
        fprintf(stderr, "rm file failed: %s", argv[1]);
        return err;
    }
    
    return 0;
}