#include "main.h"
#include "cmd_executor.h"
#include "applib/lib_syscall.h"
#include "cmd/cmd.h"
#include <string.h>
#include <sys/fcntl.h>


cli_t cli;
static char cmd_buf[256];
static const char * prompt = "[root@localhost ~] # ";

const cli_cmd_t cmd_list[] = {
    {
        .name = "help",
        .usage = "help -- list supported command",
        .do_func = do_help,
    },
    {
        .name = "clear",
        .usage = "clear: clear screen",
        .do_func = do_clear
    },
    {
        .name = "echo",
        .usage = "echo [-n count] msg",
        .do_func = do_echo
    },
    {
        .name = "exit",
        .usage = "exit from shell",
        .do_func = do_exit

    },
    {
        .name = "ls",
        .usage = "list directory",
        .do_func = do_ls
    },
    {
        .name = "ll",
        .usage = "list directory",
        .do_func = do_ls
    },
    {
        .name = "less",
        .usage = "less file",
        .do_func = do_less
    },
    {
        .name = "cp",
        .usage = "cp src dest",
        .do_func = do_cp
    },
    {
        .name = "rm",
        .usage = "rm file",
        .do_func = do_rm
    },
    {
        .name = "cat",
        .usage = "cat file",
        .do_func = do_cat
    },
    {
        .name = "date",
        .usage = "show current date",
        .do_func = do_date
    },
    {
        .name = "reboot",
        .usage = "reboot kernel",
        .do_func = do_reboot
    },
    {
        .name = "touch",
        .usage = "touch [file]",
        .do_func = do_touch
    }
};

static void cli_init(void)
{
    cli.prompt = prompt;
    memset(cli.curr_input, 0, CLI_INPUT_SIZE);

    int size = sizeof(cmd_list) / sizeof(cmd_list[0]);
    cli.cmd_start = cmd_list;
    cli.cmd_end = cmd_list + size;
    cli.size = size;
}

static void show_prompt(void)
{
    printf("%s", cli.prompt);
    fflush(stdout);
}

static int parse_arg(int* argc, char ** argv)
{
    //strchr 用于查找字符串中的一个字符，并返回该字符在字符串中第一次出现的位置
    char * cr = strchr(cli.curr_input, '\n');
    if (cr) *cr = '\0';
    cr = strchr(cli.curr_input, '\r');
    if (cr) *cr = '\0';

    //strtok 分解字符串为一组字符串。
    const char * space = " ";
    char * token = strtok(cli.curr_input, space);
    while (token && (*argc <= CLI_MAX_ARG_COUNT))
    {
        argv[(*argc)++] = token;
        token = strtok(NULL, space);
    }
    return *argc;
}

void hang()
{
    for(;;)
    {
        show_prompt();
        //fgets 从第三个参数指定的流中读取最多第二个参数大小的字符到第一个参数指定的容器地址中。
        char * str = fgets(cli.curr_input, CLI_INPUT_SIZE, stdin);

        if (!str) continue;

        int argc = 0;
        char * argv[CLI_MAX_ARG_COUNT];

        if (parse_arg(&argc, argv) != 0)
        {
            exec(argc, argv);
        }
    }
}

int main(int argc, char ** argv)
{
    int fd = open(argv[0], O_RDWR); //stdin
    dup(fd);                   //stdout
    dup(fd);                   //stderr
    
    cli_init();

    hang();
}