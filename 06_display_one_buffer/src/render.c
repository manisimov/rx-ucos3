#include "display.h"
#include <stddef.h>
#include <stdint.h>

// Pseudo random generator to make white noise effect on display.
static size_t prg_bit = 0;
static uint32_t prg_state = 123 * 1664525u + 1013904223u; // 123 - start seed

static void prg_next_state(void) {
    prg_state = prg_state * 1664525u + 1013904223u;
}

static uint8_t prg_get_bit(void) {
    uint8_t res = (prg_state >> prg_bit) & 0x01;
    prg_bit++;
    if (prg_bit > 32) {
        prg_bit = 0;
        prg_next_state();
    }
    return res;
}

// This is called once every frame.
// Use display_clear_buffer() and display_put_pixel(x, y)
// to define frame contents
void render(void) {
    static size_t coord_x = 0;
    static size_t coord_y = 0;

    display_clear_buffer();
    for (size_t y = 0; y < 64; y++) {
        for (size_t x = 0; x < 96; x++) {
            if (prg_get_bit())
                display_put_pixel(x, y);
        }
    }
}
