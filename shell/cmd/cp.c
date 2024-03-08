#include "cmd/cmd.h"
#include <stdlib.h>

int do_cp(int argc, char ** argv)
{
    if (argc < 3)
    {
        fprintf(stderr, "no [src] or no [dest]");
        return -1;
    }
    FILE * from, * to;
    from = fopen(argv[1], "rb");
    to = fopen(argv[2], "wb");
    if (!from || !to)
    {
        fprintf(stderr, "open file failed");
        goto cp_failed;
    }

    char * buf = (char*)malloc(255);
    int size;
    //fread  从给定流 stream 读取数据到 ptr 所指向的数组中。
    while ((size = fread(buf, 1, 255, from)) > 0)
    {
        fwrite(buf, 1, size, to);
    }
    free(buf);
    
cp_failed:    
    if (from)
    {
        fclose(from);
    }
    if (to)
    {
        fclose(to);
    }
    return 0;
}