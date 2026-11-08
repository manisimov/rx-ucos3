#ifndef DISPLAY_H
#define DISPLAY_H

#include <stddef.h>

void display_init(void);
void display_clear_buffer(void);
void display_put_pixel(size_t x, size_t y);

#endif // DISPLAY_H
