#include "types.h"
#include "x86.h"


//something like c

void* memset(void *dst, int c, uint n)
{
    if ((int)dst%4 == 0 && n%4 == 0)
    {
        c &= 0xFF;
        stosl(dst, (c<<24)|(c<<16)|(c<<8)|c, n/4);
    } 
    else
        stosb(dst, c, n);
    return dst;
}


int memcmp(const void *v1, const void *v2, uint n)
{
    const uchar *s1, *s2;

    s1 = v1;
    s2 = v2;
    while(n-- > 0)
    {
        if(*s1 != *s2)
            return *s1 - *s2;
        s1++, s2++;
    }

    return 0;
}

void* memmove(void *dst, const void *src, uint n)
{
    const char *s;
    char *d;

    s = src;
    d = dst;
    if(s < d && s + n > d)
    {
        s += n;
        d += n;
        while(n-- > 0)
            *--d = *--s;
    } 
    else
        while(n-- > 0)
            *d++ = *s++;

    return dst;
}

// memcpy exists to placate GCC.  Use memmove.
void* memcpy(void *dst, const void *src, uint n)
{
  return memmove(dst, src, n);
}

int strncmp(const char *p, const char *q, uint n)
{
    while(n > 0 && *p && *p == *q)
        n--, p++, q++;
    if(n == 0)
        return 0;
    return (uchar)*p - (uchar)*q;
}

int strcmp(const char* p, const char* q)
{
    while (*p && *p == *q)
        p++, q++;
    return (int) ((unsigned char) *p - (unsigned char) *q);
}

char* strncpy(char *s, const char *t, int n)
{
    char *os;
  
    os = s;
    while(n-- > 0 && (*s++ = *t++) != 0)
        ;
    while(n-- > 0)
        *s++ = 0;
    return os;
}

char * strcpy(char *dst, const char *src)
{
    char *ret;

    ret = dst;
    while ((*dst++ = *src++) != '\0')
        /* do nothing */;
    return ret;
}

// Like strncpy but guaranteed to NUL-terminate.
char* safestrcpy(char *s, const char *t, int n)
{
    char *os;
  
    os = s;
    if(n <= 0)
        return os;
    while(--n > 0 && (*s++ = *t++) != 0)
        ;
    *s = 0;
    return os;
}

int strlen(const char *s)
{
    int n;

    for(n = 0; s[n]; n++)
        ;
    return n;
}

void strsplit(char *s, char limit, char * first, char * second)
{
    char * s1 = s; 
    while(*s != limit)
        s++;
    *s = '\0';
    strcpy(first, s1);
    strcpy(second, s+1);
    return;
}

char * strcat(char *dst, const char *src)
{
    int len = strlen(dst);
    strcpy(dst + len, src);
    return dst;
}

void bzero(void *dest, uint len)
{
    memset(dest, 0, len);
}