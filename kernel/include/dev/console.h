#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include "comm/types.h"
#include "dev/tty.h"
#include "ipc/mutex.h"
#include "base/history.h"

#define CONSOLE_DISP_ADDR    0xb8000
#define CONSOLE_DISP_END     (0xb8000 + 32 * 1024)
#define CONSOLE_ROW_MAX      25
#define CONSOLE_COL_MAX      80
#define CONSOLE_NR           8

#define CRT_ADDR_PORT        0x3D4   //crt(6845)索引寄存器
#define CRT_DATA_PORT        0x3D5   //crt(6845)数据寄存器
#define CRT_START_ADDR_H     0xC     //显存起始位置 高位
#define CRT_START_ADDR_L     0XD     //显存起始位置 低位

#define ASCII_ESC           0x1b
#define ESC_PARAM_MAX       10


typedef enum _color_t {
    COLOR_Black			= 0,
    COLOR_Blue			= 1,
    COLOR_Green			= 2,
    COLOR_Cyan			= 3,
    COLOR_Red			= 4,
    COLOR_Magenta		= 5,
    COLOR_Brown			= 6,
    COLOR_Gray			= 7,
    COLOR_Dark_Gray 	= 8,
    COLOR_Light_Blue	= 9,
    COLOR_Light_Green	= 10,
    COLOR_Light_Cyan	= 11,
    COLOR_Light_Red		= 12,
    COLOR_Light_Magenta	= 13,
    COLOR_Yellow		= 14,
    COLOR_White			= 15
} color_t;


#define DEFAULT_FOREGROUND  COLOR_Gray

//显示字符
typedef union _disp_char_t
{
    struct
    {
        char c;
        char foreground : 4; //前景色
        char background : 3; //后景色
    };
    
    uint16_t v;
} disp_char_t;

typedef struct _console_t
{
    enum {
        CONSOLE_WRITE_NORMAL,
        CONSOLE_WRITE_ESC,
        CONSOLE_WRITE_SQUARE,
    } write_state;
    disp_char_t * disp_base; // 显示基地址
    int cursor_row, cursor_col;
    int display_rows, display_cols;
    color_t foreground, background;
    int old_cursor_row, old_cursor_col;

    int esc_param[ESC_PARAM_MAX];
    int curr_param_index;

    mutex_t mutex;

    int his_idx;        //能移动的历史命令索引
    int curr_his_idx;   //当前最新历史命令索引
    history_command_t his_cmds[MAX_SAVE_CMDS_NR];

    enum {
        CMD_MODE,  //命令行模式
        EDITOR_MODE,  //编辑器模式
    } console_mode; //控制台模式
} console_t;

int console_init(int idx);

int console_write(tty_t * tty);

int console_close(int console);

int console_select(int idx);

console_t * get_console(int idx);
#endif