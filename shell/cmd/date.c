#include "cmd/cmd.h"
#include <time.h>

int do_date(int argc, char ** argv)
{
    time_t timep;
    struct tm *p;
    time(&timep);
    p = localtime(&timep);

    printf("%d/%d/%d %02d:%02d:%02d\n", 
        1900 + p->tm_year, 
        1+ p->tm_mon,
        p->tm_mday, 
        p->tm_hour, 
        p->tm_min, p->tm_sec);

    return 0;
}