#include "tools/klib.h"
#include "comm/types.h"
#include "comm/cpu_instr.h"
#include "tools/log.h"


void kernel_strcpy(char * dest, const char * src)
{
    if (!dest || !src) {
        return;
    }

    while (*dest && *src) {
        *dest++ = *src++;
    }
    *dest = '\0';
}

void kernel_strncpy(char * dest, const char * src, int size)
{
    if (!dest || !src || !size) {
        return;
    }

    char * d = dest;
    const char * s = src;

    while ((size-- > 0) && (*s)) {
        *d++ = *s++;
    }
    if (size == 0) {
        *(d - 1) = '\0';
    } else {
        *d = '\0';
    }
}

int kernel_strcmp(const char * s1, const char * s2, int size)
{
    if (!s1 || !s2) {
        return -1;
    }

    while (*s1 && *s2 && (*s1 == *s2) && size) {
    	s1++;
    	s2++;
    }

    return !((*s1 == '\0') || (*s2 == '\0') || (*s1 == *s2));
}

int kernel_strlen(const char * str)
{
    if (str == (const char *)0) {
        return 0;
    }

	const char * c = str;

	int len = 0;
	while (*c++) {
		len++;
	}

	return len;
}

void kernel_memcpy(void * dest, void * src, int size)
{
    if (!dest || !src || !size) {
        return;
    }

    uint8_t * s = (uint8_t *)src;
    uint8_t * d = (uint8_t *)dest;
    while (size--) {
        *d++ = *s++;
    }
}

void kernel_memset(void * dest, uint8_t v, int size)
{
    if (!dest || !size) {
        return;
    }

    uint8_t * d = (uint8_t *)dest;
    while (size--) {
        *d++ = v;
    }
}

int kernel_memcmp(void * d1, void * d2, int size)
{
    if (!d1 || !d2) {
        return 1;
    }

	uint8_t * p_d1 = (uint8_t *)d1;
	uint8_t * p_d2 = (uint8_t *)d2;
	while (size--) {
        //只要字节内容不一样，就返回1
		if (*p_d1++ != *p_d2++) {
			return 1;
		}
	}

	return 0;
}

void kernel_sprintf(char* str_buf, const char * fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    kernel_vsprintf(str_buf, fmt, args);
    va_end(args);
}

//base 进制
void kernel_itoa(char * buf, int num, int base)
{
    static const char * num2ch = {"FEDCBA9876543210123456789ABCDEF"};
    int old_num = num;
    char * p = buf;
    if (base != 2 && base != 8 && base != 10 && base != 16) 
    {
        *p = '\0';
        return;
    }
    int signed_num = 0;
    if (num < 0 && base == 10)
    {
        *p++ = '-';
        signed_num = 1;
    }
    //正负进行分开处理。
    if(signed_num)
    {
        //数字转字符串
        do {
            char ch = num2ch[num % base + 15];
            *p++ = ch;
            num /= base;
        } while(num);
    }
    else
    {
        //对于十六进制数，不支持负数，所以需要将其转换成无符号数进行除和求余
        uint32_t u_num = (uint32_t)num;
        do {
            char ch = num2ch[u_num % base + 15];
            *p++ = ch;
            u_num /= base;
        } while (u_num);
    }

    
    *p-- = '\0';

    //字符串反转
    char * start = old_num > 0 ? buf : buf + 1;
    while (start < p)
    {
        char ch = *start;
        *start = *p;
        *p = ch;
        p--;
        start++;
    }
    
}

void kernel_vsprintf(char* buf, const char * fmt, va_list args)
{
    enum {NORMAL, READ_FMT} state = NORMAL;
    char * curr = buf;
    char ch;
    while ((ch = *fmt++))
    {
        switch (state)
        {
            case NORMAL:
                if (ch == '%')
                {
                    state = READ_FMT;
                }
                else
                {
                    *curr++ = ch;
                }
                break;
            case READ_FMT:     
                if (ch == 's')
                {
                    const char * str = va_arg(args, char*);
                    int len = kernel_strlen(str);
                    while (len -- )
                    {
                        *curr ++ = *str++;
                    }
                }
                else if (ch == 'd')
                {
                    int num = va_arg(args, int);
                    kernel_itoa(curr, num, 10);
                    curr += kernel_strlen(curr);
                }
                else if (ch == 'x')
                {
                    int num = va_arg(args, int);
                    kernel_itoa(curr, num, 16);
                    curr += kernel_strlen(curr);
                }
                else if (ch == 'c')
                {
                    char ch = va_arg(args, int);
                    *curr++ = ch;
                }           
                state = NORMAL;
                break;
            default:
                break;
        }
    }
    
}

void pannic(const char * file, int line, const char * func, const char * cond)
{
    log_printf("assert failed! cond ==> [%s]", cond);
    log_printf("file: %s, line: %d. func: %s", file, line, func);
    for(;;)
    {
        hlt();
    }
}