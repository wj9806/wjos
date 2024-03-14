#ifndef __MAIN_H__
#define __MAIN_H__

#define CLI_INPUT_SIZE              128
#define CLI_MAX_ARG_COUNT           10

//发送ESC转义序列完成清屏
//可参考dev/console.c write_esc write_esc_square 函数
#define ESC_CMD2(pn, cmd)           "\x1b["#pn#cmd
#define ESC_COLOR_ERROR             ESC_CMD2(31, m)
#define ESC_COLOR_DEFAULT           ESC_CMD2(39, m)
#define ESC_CLEAR_SCREEN            ESC_CMD2(2, J)
#define ESC_MOVE_CURSOR(row, col)   "\x1b["#row";"#col"H"    

typedef struct _cli_cmd_t
{
    const char * name;
    const char * usage;
    int (*do_func)(int argc, char** arg);

} cli_cmd_t;


typedef struct _cli_t
{
    char curr_input[CLI_INPUT_SIZE];
    const cli_cmd_t * cmd_start;
    const cli_cmd_t * cmd_end;
    const char * prompt;
    int size;
    int console_num;
} cli_t;


#endif