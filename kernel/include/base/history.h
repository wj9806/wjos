#ifndef __HISTORY_H__
#define __HISTORY_H__

#include "tools/list.h"
#include <time.h>

//最大保存的历史命令数目
#define MAX_SAVE_CMDS_NR    100

#define CLI_INPUT_SIZE              128

typedef struct _history_command_t
{
    //命令执行的时间
    time_t time;

    //命令
    char cmd[CLI_INPUT_SIZE];
} history_command_t;

void history_command_init(history_command_t * his_cmd);

void sys_save_history(int console_num, int cmd);

#endif