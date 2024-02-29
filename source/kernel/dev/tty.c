#include "dev/tty.h"
#include "dev/dev.h"
#include "tools/log.h"
#include "dev/console.h"
#include "dev/keyboard.h"

static tty_t tty_devs[TTY_NR];

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
    if (fifo->count >= fifo->size)
    {
        return -1;
    }
    fifo->buf[fifo->write++] = c;
    if (fifo->write >= fifo->size)
    {
        //指向数组的头
        fifo->write = 0;
    }
    fifo->count++;
    return 0;
}

int tty_fifo_get(tty_fifo_t * fifo, char *c)
{
    if (fifo->count <= 0)
    {
        return -1;
    }
    *c = fifo->buf[fifo->read++];
    if (fifo->read >= fifo->size)
    {
        //指向数组的头
        fifo->read = 0;
    }
    fifo->count--;
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
    tty->console_idx = idx;
    keyboard_init();
    console_init(idx);
    return 0;
}

int tty_read(device_t * dev, int addr, char * buf, int size)
{
    return size;
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
    
    int len;

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

int tty_control(device_t * dev, int cmd, int arg0, int arg1)
{
    return 0;
}

void tty_close(device_t * dev)
{
    
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