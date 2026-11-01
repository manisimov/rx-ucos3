#ifndef LOGGER_H
#define LOGGER_H

#include <stdarg.h>

void logger_init(void);
void logger_read_start(void);
int logger_read_next(void);
void logger_read_end(void);

void log_warn(const char *format, ...);

#endif // LOGGER_H
