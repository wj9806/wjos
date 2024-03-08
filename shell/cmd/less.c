#include "cmd/cmd.h"
#include "applib/lib_syscall.h"
#include "dev/tty.h"
#include <getopt.h>
#include <stdlib.h>

int do_less(int argc, char ** argv)
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
