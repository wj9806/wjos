#include "cmd/cmd.h"

int do_clear(int argc, char ** argv)
{
    printf("%s", ESC_CLEAR_SCREEN);
    printf("%s", ESC_MOVE_CURSOR(0, 0));
    return 0;
}

cli_cmd_t cli_clear = {
    .name = "clear",
    .usage = "clear: clear screen",
    .do_func = do_clear
};