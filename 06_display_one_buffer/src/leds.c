#include "iodefine.h"
#include "leds.h"
#include <stdint.h>

/* Here LED numbers are according to silkscreen on the board */

// Assume port structures are same. Because DDR and DR registers are on the same offset
struct led_info_t {
    volatile struct st_portd *port_address;
    int bit_number;
};

static const struct led_info_t leds_info[LEDS_SIZE] = {
    {&PORTD, 5}, // LED4
    {(volatile struct st_portd *)&PORTE, 3}, // LED5
    {&PORTD, 2}, // LED6
    {(volatile struct st_portd *)&PORTE, 0}, // LED7
    {&PORTD, 4}, // LED8
    {(volatile struct st_portd *)&PORTE, 2}, // LED9
    {&PORTD, 1}, // LED10
    {&PORTD, 7}, // LED11
    {&PORTD, 3}, // LED12
    {(volatile struct st_portd *)&PORTE, 1}, // LED13
    {&PORTD, 0}, // LED14
    {&PORTD, 6}, // LED15
};

void leds_init(void) {
    for (int i = 0; i < LEDS_SIZE; i++) {
        (*(leds_info[i].port_address)).DDR.BYTE |= 1U << leds_info[i].bit_number;
    }
}

void leds_toggle(unsigned int led) {
    if (led >= LEDS_SIZE)
        return;

    leds_info[led].port_address->DR.BYTE ^= 1U << leds_info[led].bit_number;
}

void leds_on(unsigned int led) {
    if (led >= LEDS_SIZE)
        return;

    leds_info[led].port_address->DR.BYTE &= ~(1U << leds_info[led].bit_number);

}

void leds_off(unsigned int led) {
    if (led >= LEDS_SIZE)
        return;

    leds_info[led].port_address->DR.BYTE |= 1U << leds_info[led].bit_number;
}
