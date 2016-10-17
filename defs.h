#ifndef DEFS_H
#define DEFS_H

//defind most function here for use

#include "types.h"
// console.c
void cprintf(char*, ...);
void cleanscreen();
void panic(char*) __attribute__((noreturn));
void kbdinit();
void consoleintr(int(*)(void));
char* readline(const char *prompt);

//entrypgtable.c
void remap();

// string.c
int memcmp(const void*, const void*, uint);
void* memmove(void*, const void*, uint);
void* memset(void*, int, uint);
char* safestrcpy(char*, const char*, int);
int strlen(const char*);
int strncmp(const char*, const char*, uint);
char* strncpy(char*, const char*, int);
int strcmp(const char*, const char*);
char* strcpy(char *, const char *);
char * strcat(char *dst, const char *src);
void strsplit(char *s, char limit, char * first, char * second);
void bzero(void *dest, uint len);

#endif