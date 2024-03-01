#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "lib_syscall.h"

char cmd_buf[256];


static cli_t cli;
static const char * prompt = "[root@localhost ~] # ";

static int do_help(int argc, char **argv)
{
    return 0;
}

static const cli_cmd_t cmd_list[] = {
    {
        .name = "help",
        .usage = "help -- list supported command",
        .do_func = do_help,
    },
};

static void cli_init(void)
{
    cli.prompt = prompt;
    memset(cli.curr_input, 0, CLI_INPUT_SIZE);

    int size = sizeof(cmd_list) / sizeof(cmd_list[0]);
    cli.cmd_start = cmd_list;
    cli.cmd_end = cmd_list + size;
}

static void show_prompt(void)
{
    printf("%s", cli.prompt);
    fflush(stdout);
}

int main(int argc, char ** argv)
{
    int fd = open(argv[0], 0); //stdin
    dup(fd);                   //stdout
    dup(fd);                   //stderr

    cli_init();

    puts("wjos start success!\n");
    for(;;)
    {
        show_prompt();
        gets(cli.curr_input);
    }
}