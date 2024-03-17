#include "base/history.h"
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