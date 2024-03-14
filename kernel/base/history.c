#include "base/history.h"
#include "dev/time.h"
#include "dev/console.h"

extern console_t console_buf[];

void history_init(history_t * history)
{
    list_init(&history->history_list);
    node_init(history->curr);
    //可从.history中加载
}

//上一个历史命令
history_command_t * pre_cmd(history_t * history)
{
    if (history != (history_t *)0)
    {
        node_t * curr = history->curr;
        if (curr == NULL)
        {
            return NULL;
        }
        if (curr->pre != NULL)
        {
            curr = curr->pre;
            return list_node_parent(curr, history_command_t, cmd_node);
        }

    }
    return NULL;
}

//下一个历史命令
history_command_t * next_cmd(history_t * history)
{
    if (history != (history_t *)0)
    {
        node_t * curr = history->curr;
        if (curr == NULL)
        {
            return NULL;
        }
        if (curr->next != NULL)
        {
            curr = curr->next;
            return list_node_parent(curr, history_command_t, cmd_node);
        }
    }
    return NULL;
}

void add(history_t * history, char * cmd)
{
    time_t timep = sys_time();

    node_t node;
    node_init(&node);

    history_command_t history_command;
    history_command.cmd = cmd;
    history_command.time = timep;
    history_command.cmd_node = node;

    if (history->history_list.count == MAX_SAVE_CMDS_NR)
    {
        list_remove_first(&history->history_list);
    }
    list_insert_last(&history->history_list, &node);
}

void sys_save_history(int console_num, int cmd)
{
    console_t console = console_buf[console_num];
    add(&console.history, (char *) cmd);
}