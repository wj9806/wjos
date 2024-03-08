#include "cmd/cmd.h"
#include "applib/lib_syscall.h"
#include <string.h>
#include "fs/file.h"

int do_ls(int argc, char ** argv)
{
    DIR * p_dir = opendir("temp");
    if (p_dir == NULL)
    {
        printf("open dir failed.\n");
        return -1;
    }
    struct dirent * entry;
    while ((entry = readdir(p_dir)) != NULL)
    {
        strlwr(entry->name);
        printf("%c %s %d\n", 
            entry->type == FILE_DIR ? 'd' : 'f', 
            entry->name, 
            entry->size);
    }
    closedir(p_dir);
    return 0;
}

cli_cmd_t cli_ls = {
    .name = "ls",
    .usage = "list directory",
    .do_func = do_ls
};