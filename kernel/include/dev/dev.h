#ifndef __DEV_H__
#define __DEV_H__

#define DEV_NAME_SIZE              32

enum {
    DEV_UNKNOWN = 0,
    DEV_TTY = 1,
    DEV_DISK = 2
};

struct _dev_desc_t;
typedef struct _device_t
{
    struct _dev_desc_t * desc;
    int mode;
    int minor; //次设备号
    void * data;
    int open_count; //打开次数
} device_t;


//描述一种类型设备
typedef struct _dev_desc_t
{
    char name[DEV_NAME_SIZE];
    int major; //主设备号

    int (*open)(device_t * dev);
    int (*read)(device_t * dev, int addr, char * buf, int size);
    int (*write)(device_t * dev, int addr, char * buf, int size);
    int (*control)(device_t * dev, int cmd, int arg0, int arg1);
    void (*close)(device_t * dev);
} dev_desc_t;

int dev_open(int major, int minor, void * data);

int dev_read(int dev_id, int addr, char * buf, int size);

int dev_write(int dev_id, int addr, char * buf, int size);

int dev_control(int dev_id, int cmd, int arg0, int arg1);

void dev_close(int dev_id);

#endif