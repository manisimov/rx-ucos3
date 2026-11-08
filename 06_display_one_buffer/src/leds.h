#ifndef LEDS_H
#define LEDS_H

enum {
    LEDS_LED4 = 0,
    LEDS_LED5,
    LEDS_LED6,
    LEDS_LED7,
    LEDS_LED8,
    LEDS_LED9,
    LEDS_LED10,
    LEDS_LED11,
    LEDS_LED12,
    LEDS_LED13,
    LEDS_LED14,
    LEDS_LED15,
    LEDS_SIZE
};

void leds_init(void);
void leds_toggle(unsigned int led);
void leds_on(unsigned int led);
void leds_off(unsigned int led);

#endif // LEDS_H
