#ifndef __HISTORY_H__
#define __HISTORY_H__

#include "list.h"
#include "main.h"
#include <time.h>

//最大保存的历史命令数目
#define MAX_SAVE_CMDS_NR    100

typedef struct _history_command_t
{
    node_t cmd_node;

    //命令执行的时间
    time_t time;

    //命令
    const char * cmd;
} history_command_t;

void history_list_init();

history_command_t * pre_cmd();

history_command_t * next_cmd();

void add(char * cmd);


#endif