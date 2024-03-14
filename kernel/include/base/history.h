#ifndef __HISTORY_H__
#define __HISTORY_H__

#include "tools/list.h"
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

typedef struct _history_t
{
    list_t history_list;

    node_t * curr;
} history_t;


void history_init(history_t * history);

history_command_t * pre_cmd(history_t * history);

history_command_t * next_cmd(history_t * history);

void add(history_t * history, char * cmd);

void sys_save_history(int console_num, int cmd);

#endif