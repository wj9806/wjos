#include "dev/console.h"
#include "comm/cpu_instr.h"
#include "tools/klib.h"

static console_t console_buf[CONSOLE_NR];

//读取当前光标的位置
static int read_cursor_pos (void) {
    int pos;

 	outb(0x3D4, 0x0F);		// 写低地址
	pos = inb(0x3D5);
	outb(0x3D4, 0x0E);		// 写高地址
	pos |= inb(0x3D5) << 8;   
    return pos;
}

//更新鼠标的位置
static void update_cursor_pos (console_t * console) {
	uint16_t pos = console->cursor_row *  console->display_cols + console->cursor_col;

	outb(0x3D4, 0x0F);		// 写低地址
	outb(0x3D5, (uint8_t) (pos & 0xFF));
	outb(0x3D4, 0x0E);		// 写高地址
	outb(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
}

//擦除从start到end的行
static void erase_rows (console_t * console, int start, int end) {
    disp_char_t * disp_start = console->disp_base + console->display_cols * start;
    disp_char_t * disp_end = console->disp_base + console->display_cols * (end + 1);

    while (disp_start < disp_end) {
        disp_start->c = ' ';
        disp_start->foreground = console->foreground;
        disp_start->background = console->background;

        disp_start++;
    }
}

//上移n行
static void scroll_up(console_t * console, int lines)
{
    // 整体上移
    disp_char_t * dest = console->disp_base;
    disp_char_t * src = console->disp_base + console->display_cols * lines;
    uint32_t size = (console->display_rows - lines) * console->display_cols * sizeof(disp_char_t);
    kernel_memcpy(dest, src, size);

    // 擦除最后一行
    erase_rows(console, console->display_rows - lines, console->display_rows - 1);

    console->cursor_row -= lines;
}

static void move_forward(console_t * console, int n)
{
    for (int i = 0; i < n; i++)
    {
        if (++console->cursor_col >= console->display_cols)
        {
            console->cursor_row++;
            console->cursor_col = 0;
            if (console->cursor_row >= console->display_rows)
            {
                scroll_up(console, 1);
            }
        }
    }
    
}

static void show_char(console_t * console, char ch)
{
    int offset = console->cursor_col + console->cursor_row * console->display_cols;
    disp_char_t * p = console->disp_base + offset;
    p->c = ch;
    p->foreground = console->foreground;
    p->background = console->background;
    //移动光标
    move_forward(console, 1);
}

//清楚屏幕
static void clear_display(console_t * console)
{
    int size = console->display_cols * console->display_rows;

    disp_char_t * start = console->disp_base;
    for (int i = 0; i < size; i++, start++)
    {
        start->c = ' ';
        start->foreground = console->foreground;
        start->background = console->background;
    }
    
}

static void move_to_col0(console_t * console)
{
    console->cursor_col = 0;
}


static void move_next_line(console_t * console)
{
    console->cursor_row++;
    if (console->cursor_row >= console->display_rows)
    {
        scroll_up(console, 1);
    }
    
}

static int move_backword(console_t * console, int n)
{
    int status = -1;
    for (int i = 0; i < n; i++)
    {
        if (console->cursor_col > 0)
        {
            console->cursor_col--;
            status = 0;
        }
        else if (console->cursor_row > 0)
        {
            console->cursor_row--;
            console->cursor_col = console->display_cols - 1;
            status = 0;
        }
    }
    
    return status;
}

static void erase_backword(console_t * console)
{
    if (move_backword(console, 1) == 0)
    {
        show_char(console, ' ');
        move_backword(console, 1);
    }
    
}

int console_init(void)
{
    for (int i = 0; i < CONSOLE_NR; i++)
    {
        console_t * console = console_buf + i;
        int cursor_pos = read_cursor_pos();

        console->background =COLOR_Black;
        console->foreground = COLOR_Gray;
        console->display_cols = CONSOLE_COL_MAX;
        console->display_rows = CONSOLE_ROW_MAX;

        console->cursor_col = cursor_pos % console->display_cols;
        console->cursor_row = cursor_pos / console->display_cols;
        console->old_cursor_row = console->cursor_row;
        console->old_cursor_col = console->cursor_col;

        console->disp_base = (disp_char_t *)CONSOLE_DISP_ADDR + i * (CONSOLE_COL_MAX * CONSOLE_ROW_MAX);
        //clear_display(console);
    }
   
    return 0;
}

static void write_normal(console_t * c, char ch)
{
    switch (ch)
    {
        case ASCII_ESC:
            c->write_state = CONSOLE_WRITE_ESC;
            break;
        case 0x7f:
            //往回删除一个字符
            erase_backword(c);
            break;
        case '\b':
            //往左移动一个光标
            move_backword(c, 1);
            break;
        case '\r':
        move_to_col0(c);
            break;
        case '\n':
            move_to_col0(c);
            move_next_line(c);
            break;
        default:
            if ((ch >= ' ') && (ch <= '~'))
            {
                show_char(c, ch);
            }
            break;
    }
}

void save_cursor(console_t * console)
{
    console->old_cursor_col = console->cursor_col;
    console->old_cursor_row = console->cursor_row;
}

void restore_cursor(console_t * console)
{
    console->cursor_col = console->old_cursor_col;
    console->cursor_row = console->old_cursor_row;
}

static void write_esc(console_t * c, char ch)
{
    switch (ch)
    {
    case '7':
        save_cursor(c);
        c->write_state = CONSOLE_WRITE_NORMAL;
        break;
    case '8':
        restore_cursor(c);
        c->write_state = CONSOLE_WRITE_NORMAL;
    default:
        c->write_state = CONSOLE_WRITE_NORMAL;
        break;
    }
}

int console_write(int console, char * data, int size)
{
    console_t * c = console_buf + console;
    int len;
    for (int len = 0; len < size; len++)
    {
        //取出字符
        char ch = *data++;
        switch (c->write_state)
        {
        case CONSOLE_WRITE_NORMAL:
            write_normal(c, ch);
            break;
        case CONSOLE_WRITE_ESC:
            write_esc(c, ch);
            break;
        default:
            break;
        }
    }
    update_cursor_pos(c);
    return len;
}

int console_close(int console)
{

}