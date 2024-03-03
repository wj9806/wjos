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
    const cli_cmd_t * start = cli.cmd_start;
    while (start < cli.cmd_end)
    {
        printf("%s %s\n", start->name, start->usage);
        start++;
    }
    
    return 0;
}

static int do_clear(int argc, char ** argv)
{
    printf("%s", ESC_CLEAR_SCREEN);
    printf("%s", ESC_MOVE_CURSOR(0, 0));
    return 0;
}

static const cli_cmd_t cmd_list[] = {
    {
        .name = "help",
        .usage = "help -- list supported command",
        .do_func = do_help,
    },
    {
        .name = "clear",
        .usage = "clear: clear screen",
        .do_func = do_clear
    }
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

static const cli_cmd_t * find_builtin(const char * name) 
{
    for (const cli_cmd_t * cmd = cli.cmd_start; cmd < cli.cmd_end; cmd++)
    {
        if (strcmp(cmd->name, name) != 0)
        {
            continue;
        }

        return cmd;
    }
    return (cli_cmd_t *) 0;
}

static void run_builtin(const cli_cmd_t * cmd, int argc, char ** argv)
{
    int ret = cmd->do_func(argc, argv);
    if (ret < 0)
    {
        fprintf(stderr, ESC_COLOR_ERROR"error: %d\n"ESC_COLOR_DEFAULT, ret);
    }
    
}

int main(int argc, char ** argv)
{
    int fd = open(argv[0], 0); //stdin
    dup(fd);                   //stdout
    dup(fd);                   //stderr

    cli_init();

    for(;;)
    {
        show_prompt();
        //fgets 从第三个参数指定的流中读取最多第二个参数大小的字符到第一个参数指定的容器地址中。
        char * str = fgets(cli.curr_input, CLI_INPUT_SIZE, stdin);
        if (!str)
        {
            continue;
        }
        //strchr 用于查找字符串中的一个字符，并返回该字符在字符串中第一次出现的位置
        char * cr = strchr(cli.curr_input, '\n');
        if (cr) *cr = '\0';
        cr = strchr(cli.curr_input, '\r');
        if (cr) *cr = '\0';

        int argc = 0;
        char * argv[CLI_MAX_ARG_COUNT];

        //strtok 分解字符串为一组字符串。
        const char * space = " ";
        char * token = strtok(cli.curr_input, space);
        while (token && (argc <= CLI_MAX_ARG_COUNT))
        {
            argv[argc++] = token;
            token = strtok(NULL, space);
        }
        if (argc == 0)
        {
            continue;
        }
        
        const cli_cmd_t * cmd = find_builtin(argv[0]);
        if (cmd)
        {
            run_builtin(cmd, argc, argv);
            continue;
        }
        //磁盘加载

        fprintf(stderr, ESC_COLOR_ERROR"%s: command not found\n"ESC_COLOR_DEFAULT, cli.curr_input);
    }
}