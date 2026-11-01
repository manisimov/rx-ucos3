#include <iodefine.h>
#include <stdint.h>
#include "hw.h"

#define EXTERNAL_CLOCK_FREQ (12000000UL)

void hw_init(void) {
    SYSTEM.SCKCR.LONG = 0x00020100;    /* Clocking = 12MHz                   */
                                       /* I Clock  = 96MHz, B Clock = 24MHz, */
                                       /* P Clock  = 48MHz                   */
}

uint32_t hw_get_pclock_freq(void) {
    uint32_t sys_clk;
    uint32_t per_clk;
    uint32_t per_div;

    sys_clk =  SYSTEM.SCKCR.LONG;
    per_div = (sys_clk >> 8u) & 0xFu;
    if (per_div > 3u) {
        return (0u);
    }

    per_clk = EXTERNAL_CLOCK_FREQ << (3u - per_div);

    return (per_clk);
}
