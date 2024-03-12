#include "cmd_executor.h"
#include "main.h"
#include "applib/lib_syscall.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>

extern cli_t cli;

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
    static char path[255];

    int fd = open(filename, 0);
    if (fd < 0) {
        sprintf(path, "%s.elf", filename);
        fd = open(path, 0);
        if (fd < 0) {
            return (const char * )0;
        }
    }

    close(fd);
    return path;
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

void exec(int argc, char ** argv)
{
    const cli_cmd_t * cmd = find_builtin(argv[0]);
    if (cmd)
    {
        run_builtin(cmd, argc, argv);
        return;
    }

    const char * path = find_exec_path(argv[0]);
    if (path)
    {
        //磁盘加载
        run_exec_file(path, argc, argv);
        return;
    }

    fprintf(stderr, ESC_COLOR_ERROR"%s: command not found\n"ESC_COLOR_DEFAULT, cli.curr_input);
}

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