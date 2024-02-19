#ifndef __KLIB_H__
#define __KLIB_H__

#include "comm/types.h"
#include<stdarg.h>

void kernel_strcpy(char * dest, const char * src);

void kernel_strncpy(char * dest, const char * src, int size);

int kernel_strcmp(const char * s1, const char * s2, int size);

int kernel_strlen(const char * str);

void kernel_memcpy(void * dest, void * src, int size);

void kernel_memset(void * dest, uint8_t src, int size);

int kernel_memcmp(void * d1, void * d2, int size);

void kernel_sprintf(char* str_buf, const char * fmt, ...);

void kernel_vsprintf(char* str_buf, const char * fmt, va_list args);

#ifndef RELEASE
#define ASSERT(expr) \
    if (!(expr)) pannic(__FILE__, __LINE__, __func__, #expr);
    
void pannic(const char * file, int line, const char * func, const char * cond);
#else
#define ASSERT(expr) ((void)0)
#endif //RELEASE

#endif