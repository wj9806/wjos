#include "cmd/history.h"

static list_t history_list;
static history_command_t curr;


void history_list_init()
{
    list_init(&history_list);
    //可从.history中加载
}

history_command_t * pre_cmd()
{

}

history_command_t * next_cmd()
{

}

void add(char * cmd)
{
    time_t timep;
    time(&timep);

    node_t node;
    node_init(&node);

    history_command_t history;
    history.cmd = cmd;
    history.time = timep;
    history.cmd_node = node;

    list_insert_first(&history_list, &node);
}