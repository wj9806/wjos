#include "cmd/cmd.h"
#include "applib/lib_syscall.h"
#include "core/sys.h"

int do_reboot(int argc, char ** argv)
{
    return power(SYS_REBOOT);
}