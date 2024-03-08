#include "cmd/cmd.h"
#include <getopt.h>
#include <stdlib.h>

int do_echo(int argc, char ** argv)
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

cli_cmd_t cli_echo = {
    .name = "echo",
    .usage = "echo [-n count] msg",
    .do_func = do_echo
};
