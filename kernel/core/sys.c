#include "core/sys.h"
#include "comm/cpu_instr.h"

int sys_power(int status)
{
    if (status == SYS_REBOOT)
    {
        io_out8(0x64,0xFE);
    }
    else if (status == SYS_SHUTDOWN)
    {
        /* code */
    }
    
    return 0;
}