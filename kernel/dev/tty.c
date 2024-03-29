#include "dev/tty.h"
#include "dev/dev.h"
#include "tools/log.h"
#include "dev/console.h"
#include "dev/keyboard.h"
#include "cpu/irq.h"

static tty_t tty_devs[TTY_NR];

static int curr_tty = 0;

static tty_t * get_tty(device_t * device)
{
    int tty = device->minor;
    if ((tty < 0) || (tty >= TTY_NR) || (!device->open_count))
    {
        log_printf("get_tty failed, tty=%d, open_count=%d", tty, device->open_count);
        return (tty_t *) 0;
    }

    return tty_devs + tty;
}

static void tty_fifo_init(tty_fifo_t * fifo, char * buf, int size)
{
    fifo->buf = buf;
    fifo->count = 0;
    fifo->size = size;
    fifo->read = fifo->write = 0;
}

int tty_fifo_put(tty_fifo_t * fifo, char c)
{
    irq_state_t state = irq_enter_protection();
    if (fifo->count >= fifo->size)
    {
        irq_leave_protection(state);
        return -1;
    }
    fifo->buf[fifo->write++] = c;
    if (fifo->write >= fifo->size)
    {
        //指向数组的头
        fifo->write = 0;
    }
    fifo->count++;
    irq_leave_protection(state);
    return 0;
}

int tty_fifo_get(tty_fifo_t * fifo, char *c)
{
    irq_state_t state = irq_enter_protection();
    if (fifo->count <= 0)
    {
        irq_leave_protection(state);
        return -1;
    }
    *c = fifo->buf[fifo->read++];
    if (fifo->read >= fifo->size)
    {
        //指向数组的头
        fifo->read = 0;
    }
    fifo->count--;
    irq_leave_protection(state);
    return 0;
}

int tty_open(device_t * dev) 
{
    int idx = dev->minor;
    if ((idx <0) || (idx >= TTY_NR))
    {
        log_printf("open tty failed, num = %d", idx);
        return -1;
    }
    
    tty_t * tty = tty_devs + idx;

    tty_fifo_init(&tty->ofifo, tty->obuf, TTY_OBUF_SIZE);
    tty_fifo_init(&tty->ififo, tty->ibuf, TTY_IBUF_SIZE);

    sem_init(&tty->osem, TTY_OBUF_SIZE);
    sem_init(&tty->isem, 0);
    
    tty->oflags = TTY_OCRLF;
    tty->iflags = TTY_INCLR | TTY_IECHO;
    tty->console_idx = idx;
    keyboard_init();
    console_init(idx);
    return 0;
}

int tty_write(device_t * dev, int addr, char * buf, int size)
{
    if (size < 0)
    {
        return -1;
    }
    
    tty_t * tty = get_tty(dev);
    if (!tty)
    {
        return -1;
    }
    
    int len = 0;

    while (size)
    {
        char c = *buf++;

		// 如果遇到\n，根据配置决定是否转换成\r\n
		if (c == '\n' && (tty->oflags & TTY_OCRLF)) {
			sem_wait(&tty->osem);
			int err = tty_fifo_put(&tty->ofifo, '\r');
			if (err < 0) {
				break;
			}
		}

        sem_wait(&tty->osem);
        int err = tty_fifo_put(&tty->ofifo, c);
        if (err < 0)
        {
            break;
        }
        len++;
        size--;

        console_write(tty);
    }
    
    return len;
}

int tty_read(device_t * dev, int addr, char * buf, int size)
{
    if (size < 0)
    {
        return -1;
    }
    
    tty_t * tty = get_tty(dev);
    char * pbuf = buf;

    int len = 0;
    while (len < size)
    {
        sem_wait(&tty->isem);

        char ch;
        tty_fifo_get(&tty->ififo, &ch);
        
        console_t * curr_console = get_console(tty->console_idx);
        switch (ch)
        {
            //退格
            case 0x7f:
                if (len == 0)
                {
                    continue;
                }
                len--;
                pbuf--;
                curr_console->input_len--;
                break;
                //换行
            case '\n':
                if ((tty->iflags) && (len < size - 1))
                {
                    *pbuf++='\r';
                    len++;
                }
                *pbuf++='\n';
                len++;
                curr_console->input_len = 0;
                break;
            default:
                *pbuf++=ch;
                len++;
                curr_console->input_len++;
                break;
        }
        if (tty->iflags & TTY_IECHO)
        {
            tty_write(dev, 0, &ch, 1);
        }
        
        if ((ch == '\n') || (ch == '\r'))
        {
            break;
        }
        
    }
    
    return len;
}

int tty_control(device_t * dev, int cmd, int arg0, int arg1)
{
    tty_t * tty = get_tty(dev);
    switch (cmd)
    {
    case TTY_CMD_ECHO:
        if (arg0)
        {
            //设置成1
            tty->iflags |= TTY_IECHO;
        }
        else
        {
            //设置成0
            tty->iflags &= ~TTY_IECHO;
        }
        break;
    
    default:
        break;
    }
    return 0;
}

void show_his_cmd(console_t * console, history_command_t * his_cmd)
{
    erase_backword(console, console->input_len);

    char ch;
    for (int i = 0; i < CLI_INPUT_SIZE; i++)
    {
        ch = his_cmd->cmd[i];
        if (ch != '\0')
        {
            tty_in(ch);
        }
        else
        {
            break;
        }
    }
}

void tty_handle_key(int key)
{
    console_t * console = get_console(curr_tty);
    if (console->console_mode == CMD_MODE)
    {
        history_command_t * his_cmd;
        if (key == KEY_UP)
        {
            his_cmd = pre_cmd(console);
            if (his_cmd->cmd[0] == '\0')
            {
                return;
            }
            
        }
        else if (key == KEY_DOWN)
        {
            his_cmd = next_cmd(console);
            if (his_cmd->cmd[0] == '\0')
            {
                return;
            }
        }
        else 
        {
            return;
        }
        show_his_cmd(console, his_cmd);
    }
    
}

void tty_close(device_t * dev)
{
    
}

void tty_select(int tty)
{
    if (tty != curr_tty)
    {
        console_select(tty);
        curr_tty = tty;
    }
    
}

void tty_in(char ch)
{
    tty_t * tty = tty_devs + curr_tty;
    if (sem_count(&tty->isem) >= TTY_IBUF_SIZE)
    {
        return;
    }
    tty_fifo_put(&tty->ififo, ch);

    sem_notify(&tty->isem);
}

dev_desc_t dev_tty_desc = {
    .name = "tty",
    .major = DEV_TTY,
    .open = tty_open,
    .read = tty_read,
    .write = tty_write,
    .control = tty_control,
    .close = tty_close,
};