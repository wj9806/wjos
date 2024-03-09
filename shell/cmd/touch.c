#include "cmd/cmd.h"
#include <stdlib.h>

int do_touch(int argc, char ** argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "no [file]");
        return -1;
    }

    FILE * f;
    f = fopen(argv[1], "wb");
    if (!f)
    {
        fprintf(stderr, "touch failed: %s", argv[1]);
        goto touch_failed;
    }

touch_failed:
    if (f)
    {
        fclose(f);
    }    

    return 0;
}