#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/file.h>
#include "main.h"
#include "lib_syscall.h"
#include "fs/file.h"
#include "dev/tty.h"

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
    int n = 0;
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
                n = 1;
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

    if (n == 1)
    {
        // 循环打印消息
        char * msg = argv[optind];
        for (int i = 0; i < count; i++) {
            puts(msg);
        }
    }
    else
    {
        for (int i = 1; i < argc; i++)
        {
            printf(argv[i], "");
            printf(" ");
        }
        printf("\n");
    }

    optind = 1;
    return 0;
}

static int do_exit(int argc, char ** argv)
{
    exit(0);
    return 0;
}

static int do_ls(int argc, char ** argv)
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

static int do_less(int argc, char ** argv)
{
    int line_mode = 0;
    int ch;
    while ((ch = getopt(argc, argv, "lh")) != -1) {
        switch (ch) {
            case 'h':
                puts("show file content");
                puts("less [-l] file");
                puts("-l show file line by line.");
                break;
            case 'l':
                line_mode = 1;
                break;
            case '?':
                if (optarg) {
                    fprintf(stderr, "Unknown option: -%s\n", optarg);
                }
                optind = 1;        // getopt需要多次调用，需要重置
                return -1;
        }
    }
    // 索引已经超过了最后一个参数的位置，意味着没有传入要发送的信息
    if (optind > argc - 1) {
        fprintf(stderr, "no file\n");
        optind = 1;        // getopt需要多次调用，需要重置
        return -1;
    }
    FILE * file = fopen(argv[optind], "r");
    if (file == NULL) {
        fprintf(stderr, "open file failed. %s", argv[optind]);
        optind = 1;        // getopt需要多次调用，需要重置
        return -1;
    }

    char * buf = (char *)malloc(255);
    if (line_mode == 0)
    {
        while (fgets(buf, 255, file) != NULL)  {
            fputs(buf, stdout);
        }
    }
    else
    {
        //设置输入无缓存
        setvbuf(stdin, NULL, _IONBF, 0);
        ioctl(0, TTY_CMD_ECHO, 0, 0);
        while (1)
        {
            char * b = fgets(buf, 255, file);
            if (b == NULL)
            {
                break;
            }
            fputs(buf, stdout);
            int ch;
            while ((ch = fgetc(stdin)) != 'n') {
                if (ch == 'q') {
                    goto less_quit;
                }
            }
        }
less_quit:   
        setvbuf(stdin, NULL, _IOLBF, BUFSIZ);
        ioctl(0, TTY_CMD_ECHO, 1, 0);
    }

    free(buf);
    fclose(file);
    optind = 1;        // getopt需要多次调用，需要重置
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

    },
    {
        .name = "ls",
        .usage = "list directory",
        .do_func = do_ls
    },
    {
        .name = "less",
        .usage = "less file",
        .do_func = do_less
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

static const char * find_exec_path(const char * filename)
{
    int fs = open(filename, 0);
    if (fs < 0)
    {
        return (const char *)0;
    }
    close(fs);
    return filename;
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
            int err = execve(path, argv, (char * const *)0);
            if (err < 0)
            {
                fprintf(stderr, "exec failed %s", path);
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

        const char * path = find_exec_path(argv[0]);
        if (path)
        {
            //磁盘加载
            run_exec_file(path, argc, argv);
            continue;
        }

        fprintf(stderr, ESC_COLOR_ERROR"%s: command not found\n"ESC_COLOR_DEFAULT, cli.curr_input);
    }
}