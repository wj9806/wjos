#include "base/history_cmd.h"
#include "dev/time.h"
#include "dev/console.h"
#include "tools/klib.h"

void history_command_init(history_command_t * his_cmd)
{
    his_cmd->time = 0;
    kernel_memset(his_cmd->cmd, '\0', CLI_INPUT_SIZE);
}

void add(console_t * console, char * cmd)
{
    //判断是否够与上一个命令相同，如果一样则只更新时间
    time_t timep = sys_time();

    history_command_t * pre = peek_pre_cmd(console);
    
    if (!kernel_all_strcmp(pre->cmd, cmd))
    {
        pre->time = timep;
        return;
    }

    int curr_his_idx = ++console->his_idx;
    if(curr_his_idx == MAX_SAVE_CMDS_NR)
    {
        //初始索引从0开始
       console->his_idx = curr_his_idx = 0;
    }

    console->curr_his_idx = -1;
    history_command_t * his_cmd = &console->his_cmds[curr_his_idx];
    history_command_init(his_cmd);

    his_cmd->time = timep;

    kernel_memcpy(his_cmd->cmd, cmd, kernel_strlen(cmd));
}

void sys_save_history(int console_num, int cmd)
{
    console_t * console = get_console(console_num);
    add(console, (char *) cmd);
}

static int history_count(console_t * console)
{
    history_command_t * cmds = console->his_cmds;
    int idx = console->his_idx;
    if (cmds[MAX_SAVE_CMDS_NR-1].cmd[0] != '\0')
    {
        return MAX_SAVE_CMDS_NR;
    }
    else
    {
        if (idx == MAX_SAVE_CMDS_NR-1)
        {
            return 0;
        }
    }
    return idx + 1;
}

void sys_get_history(int console_num, char ** cmds)
{

    console_t * console = get_console(console_num);
    int cnt = history_count(console);

    kernel_memset(cmds, '\0', sizeof(cmds));

    int head = console->his_cmds[MAX_SAVE_CMDS_NR-1].cmd[0] != '\0' 
        ? (console->his_idx + 1 == MAX_SAVE_CMDS_NR ? 0 : console->his_idx + 1) 
        : 0;
    if (head == 0)
    {
        for (int i = 0; i < cnt; i++)
        {
            char * src = console->his_cmds[i].cmd;
            kernel_strncpy(cmds[i], src, kernel_strlen(src));
        }
    }
    else
    {
        int s = 0;
        for (int i = head; i < MAX_SAVE_CMDS_NR; i++)
        {
            char * src = console->his_cmds[i].cmd;
            kernel_strncpy(cmds[s++], src, kernel_strlen(src));
        }
        for (int i = 0; i < head; i++)
        {
            char * src = console->his_cmds[i].cmd;
            kernel_strncpy(cmds[s++], src, kernel_strlen(src));
        }
    }
}