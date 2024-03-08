#include "cmd/cmd.h"
#include <stdlib.h>

int do_cat(int argc, char ** argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "cat invalid param");
        return -1;
    }
    FILE * file = fopen(argv[argc-1], "r");

    char * buf = (char *)malloc(255);
    while (fgets(buf, 255, file) != NULL)  {
        fputs(buf, stdout);
    }
    fclose(file);
    return 0;
}
