#include "cmd/cmd.h"

int do_help(int argc, char **argv)
{
    const cli_cmd_t * start = cli.cmd_start;
    while (start < cli.cmd_end)
    {
        printf("%s %s\n", start->name, start->usage);
        *start++;
    }
 
    return 0;
}

cli_cmd_t cli_help = {
    .name = "help",
    .usage = "help -- list supported command",
    .do_func = do_help,
};
