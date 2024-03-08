#include <stdio.h>
#include <time.h>
#include "applib/lib_syscall.h"

int main(int argc, char ** argv)
{
    time_t timep;
    struct tm *p;
    while (1)
    {
        time(&timep);
        printf("%ld\n", timep);

        
        p = localtime(&timep);

        printf("%d/%d/%d %02d:%02d:%02d\n", 
            1900 + p->tm_year, 
            1+ p->tm_mon,
            p->tm_mday, 
            p->tm_hour, 
            p->tm_min, p->tm_sec);

        sleep(1000);
    }

    return 0;
}