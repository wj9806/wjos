#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/file.h>
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

static int do_echo(int argc, char ** argv)
{
    if (argc == 1)
    {
        char msg_buf[128];
        fgets(msg_buf, sizeof(msg_buf), stdin);
        msg_buf[sizeof(msg_buf) - 1] = '\0';
        puts(msg_buf);
        return 0;
    }
    
    int count = 1;
    int ch;
    while ((ch = getopt(argc, argv, "n:h")) != -1)
    {
        switch (ch)
        {
            //echo -h
            case 'h':
                puts("echo any message");
                puts("Usage: echo [-n count] message");
                // getopt需要多次调用，需要重置
                optind = 1;
                return 0;
            case 'n':
                //atoi 把字符串转换成整型数。
                count = atoi(optarg);
                break;
            case '?':
                if (optarg)
                {
                    fprintf(stderr, "Unknown option: -%s", optarg);
                }
                optind = 1;
                return -1;
            default:
                break;
        }
    }
    // 索引已经超过了最后一个参数的位置，意味着没有传入要发送的信息
    if (optind > argc -1)
    {
        fprintf(stderr, "Message is empty \n");
        optind = 1;
        return -1;
    }
        // 循环打印消息
    char * msg = argv[optind];
    for (int i = 0; i < count; i++) {
        
        puts(msg);
    }
    optind = 1;
    return 0;
}

static int do_exit(int argc, char ** argv)
{
    exit(0);
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

static void run_exec_file(const char * path, int argc, char ** argv)
{
    int pid = fork();
        if (pid < 0)
        {
            fprintf(stderr, "fork failed %s", path);
        }
        else if (pid == 0)
        {   
            for (int i = 0; i < argc; i++)
            {
                printf("arg %s\n", argv[i]);
            }
            exit(-1);
        }
        else
        {
            int status;
            //wait函数等待子进程的退出
            int pid = wait(&status);
            fprintf(stderr, "cmd %s result = %d, pid = %d\n", path, status, pid);
        }
}

int main(int argc, char ** argv)
{
    int fd = open(argv[0], O_RDWR); //stdin
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
        run_exec_file("", argc, argv);

        fprintf(stderr, ESC_COLOR_ERROR"%s: command not found\n"ESC_COLOR_DEFAULT, cli.curr_input);
    }
}