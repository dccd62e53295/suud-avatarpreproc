
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

#include "log4c.h"

void log4c_raw(char const *filename, int line, char const *format, ...) {
printf("%lu[%s:%d] ",time(NULL), filename, line);
va_list vas;
va_start(vas, format);
vprintf(format, vas);
va_end(vas);
printf("\n");
};
