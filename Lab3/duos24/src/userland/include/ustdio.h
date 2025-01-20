#ifndef __USTDIO_H
#define __USTDIO_H
#include <unistd.h>
#include <stdint.h>
#include <stdarg.h>
void uprintf(char *format,...);
void uscanf(char *format, ...);  /* Read formatted input */
#endif