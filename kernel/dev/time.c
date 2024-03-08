#include "dev/time.h"
#include "tools/log.h"
#include "comm/cpu_instr.h"
#include "dev/clock.h"
#include "os_cfg.h"

// 每个月开始时的已经过去天数
static int month[13] = {
    0, // 这里占位，没有 0 月，从 1 月开始
    0,
    (31),
    (31 + 29),
    (31 + 29 + 31),
    (31 + 29 + 31 + 30),
    (31 + 29 + 31 + 30 + 31),
    (31 + 29 + 31 + 30 + 31 + 30),
    (31 + 29 + 31 + 30 + 31 + 30 + 31),
    (31 + 29 + 31 + 30 + 31 + 30 + 31 + 31),
    (31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30),
    (31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31),
    (31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30)};

//单位秒
time_t startup_time;
int century;

// 读 cmos 寄存器的值
static uint8_t cmos_read(uint8_t addr)
{
    outb(CMOS_ADDR, CMOS_NMI | addr);
    return inb(CMOS_DATA);
};

// 写 cmos 寄存器的值
static void cmos_write(uint8_t addr, uint8_t value)
{
    outb(CMOS_ADDR, CMOS_NMI | addr);
    outb(CMOS_DATA, value);
}

// 将 bcd 码转成整数
static uint8_t bcd_to_bin(uint8_t value)
{
    return (value & 0xf) + (value >> 4) * 10;
}

// 将整数转成 bcd 码
static uint8_t bin_to_bcd(uint8_t value)
{
    return (value / 10) * 0x10 + (value % 10);
}

static int get_yday(tm_t *time)
{
    int res = month[time->tm_mon]; // 已经过去的月的天数
    res += time->tm_mday;          // 这个月过去的天数

    int year;
    if (time->tm_year >= 70)
        year = time->tm_year - 70;
    else
        year = time->tm_year - 70 + 100;

    // 如果不是闰年，并且 2 月已经过去了，则减去一天
    // 注：1972 年是闰年，这样算不太精确，忽略了 100 年的平年
    if ((year + 2) % 4 && time->tm_mon > 2)
    {
        res -= 1;
    }

    return res;
}

void time_init(void)
{
    tm_t time;
    time_read(&time);
    startup_time = sys_mktime(&time);
    log_printf("startup time: %d%d-%d-%d %d:%d:%d",
        century,
        time.tm_year,
        time.tm_mon,
        time.tm_mday,
        time.tm_hour,
        time.tm_min,
        time.tm_sec);
}

void time_read_bcd(tm_t *time)
{
    // CMOS 的访问速度很慢。为了减小时间误差，在读取了下面循环中所有数值后，
    // 若此时 CMOS 中秒值发生了变化，那么就重新读取所有值。
    // 这样内核就能把与 CMOS 的时间误差控制在 1 秒之内。
    do
    {
        time->tm_sec = cmos_read(CMOS_SECOND);
        time->tm_min = cmos_read(CMOS_MINUTE);
        time->tm_hour = cmos_read(CMOS_HOUR);
        time->tm_wday = cmos_read(CMOS_WEEKDAY);
        time->tm_mday = cmos_read(CMOS_DAY);
        time->tm_mon = cmos_read(CMOS_MONTH);
        time->tm_year = cmos_read(CMOS_YEAR);
        century = cmos_read(CMOS_CENTURY);
    } while (time->tm_sec != cmos_read(CMOS_SECOND));
}

void time_read(tm_t *time)
{
    time_read_bcd(time);
    time->tm_sec = bcd_to_bin(time->tm_sec);
    time->tm_min = bcd_to_bin(time->tm_min);
    time->tm_hour = bcd_to_bin(time->tm_hour);
    time->tm_wday = bcd_to_bin(time->tm_wday);
    time->tm_mday = bcd_to_bin(time->tm_mday);
    time->tm_mon = bcd_to_bin(time->tm_mon);
    time->tm_year = bcd_to_bin(time->tm_year);
    time->tm_yday = get_yday(time);
    time->tm_isdst = -1;
    century = bcd_to_bin(century);
}

time_t sys_mktime(tm_t *time)
{
    time_t res;
    int year; // 1970 年开始的年数
    // 下面从 1900 年开始的年数计算
    if (time->tm_year >= 70)
        year = time->tm_year - 70;
    else
        year = time->tm_year - 70 + 100;

    // 这些年经过的秒数时间
    res = YEAR * year;

    // 已经过去的闰年，每个加 1 天
    res += DAY * ((year + 1) / 4);

    // 已经过完的月份的时间
    res += month[time->tm_mon] * DAY;

    // 如果 2 月已经过了，并且当前不是闰年，那么减去一天
    if (time->tm_mon > 2 && ((year + 2) % 4))
        res -= DAY;

    // 这个月已经过去的天
    res += DAY * (time->tm_mday - 1);

    // 今天过去的小时
    res += HOUR * time->tm_hour;

    // 这个小时过去的分钟
    res += MINUTE * time->tm_min;

    // 这个分钟过去的秒
    res += time->tm_sec;

    return res;
}

int sys_gettimeofday(struct timeval* tv, timezone * tz)
{
    time_t t = sys_time();
    tv->tv_sec = t;
    tv->tv_usec =  (get_sys_tick() * (1000 / OS_TICKS_MS)) % 1000;
    //暂不考虑时区
    return 0;
}

time_t sys_time()
{
    return startup_time + (get_sys_tick() * OS_TICKS_MS) / 1000;
}

uint32_t fml_stamp_to_time(long timep, struct tm * date)
{
    uint32_t days = timep / 86400;
    uint32_t rem = timep % 86400;
 
    // 计算年份
    uint16_t year;
    for (year = 1970; ; ++year)
    {
        uint16_t leap = ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0);
        uint16_t ydays = leap ? 366 : 365;
        if (days < ydays)
        {
            break;
        }
        days -= ydays;
    }
    date->tm_year = year - 1900;
    date->tm_yday = days;
    date->tm_wday = date->tm_yday % 7 + 1;
    // 计算月份
    static const uint16_t days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    uint16_t month;
    for (month = 0; month < 12; month++)
    {
        uint16_t mdays = days_in_month[month];
        if (month == 1 && ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0))
        {
            mdays = 29;
        }
        if (days < mdays)
        {
            break;
        }
        days -= mdays;
    }
    date->tm_mon = month;
 
    // 计算日期
    date->tm_mday = days + 1;
 
    // 计算时间
    date->tm_hour = rem / 3600;
    rem %= 3600;
    date->tm_min = rem / 60;
    date->tm_sec = rem % 60;
    date->tm_isdst = 0;
    return 0;
}


int sys_gmtime_r(const time_t * timep, struct tm * result)
{
    long t = (long)(*timep);
    fml_stamp_to_time(t, result);
    return (int)result;
}
