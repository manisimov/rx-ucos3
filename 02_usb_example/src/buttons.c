#include "iodefine.h"
#include "buttons.h"
#include <stdint.h>

enum {
    HYST_STATE_RELEASED,
    HYST_STATE_PRESSED
}; // internal state

enum {
    BUTTON_RELEASED,
    BUTTON_SHORT_PRESS,
};

static int button_states[BUTTON_SIZE];
static int button_presses[BUTTON_SIZE];
static uint16_t button_filters[BUTTON_SIZE];

#define SWITCH1 PORT4.PORT.BIT.B0 // left
#define SWITCH2 PORT4.PORT.BIT.B1 // middle
#define SWITCH3 PORT4.PORT.BIT.B2 // right

#define SWITCH1_DDR PORT4.DDR.BIT.B0
#define SWITCH2_DDR PORT4.DDR.BIT.B1
#define SWITCH3_DDR PORT4.DDR.BIT.B2

#define SWITCH1_ICR PORT4.ICR.BIT.B0
#define SWITCH2_ICR PORT4.ICR.BIT.B1
#define SWITCH3_ICR PORT4.ICR.BIT.B2

static void process_buttons(void) {
    for (int i = 0; i < BUTTON_SIZE; i++) {
        if (button_states[i] == HYST_STATE_RELEASED && button_filters[i] == 0xFFFFU) {
            button_states[i] = HYST_STATE_PRESSED;
        }
        if (button_states[i] == HYST_STATE_PRESSED && button_filters[i] == 0x0U) {
            button_states[i] = HYST_STATE_RELEASED;
            button_presses[i] = BUTTON_SHORT_PRESS;
        }
    }
}

void buttons_init(void) {
    SWITCH1_DDR = 0;
    SWITCH1_ICR = 1;
    SWITCH2_DDR = 0;
    SWITCH2_ICR = 1;
    SWITCH3_DDR = 0;
    SWITCH3_ICR = 1;
}

void buttons_task(void) {
    // Filter raw switch state value

    if (SWITCH1) {
        // released
        button_filters[BUTTON_LEFT] = (button_filters[BUTTON_LEFT] << 1) & 0xFFFEU;
    } else {
        // pressed
        button_filters[BUTTON_LEFT] = (button_filters[BUTTON_LEFT] << 1) | 1U;
    }

    if (SWITCH2) {
        // released
        button_filters[BUTTON_MIDDLE] = (button_filters[BUTTON_MIDDLE] << 1) & 0xFFFEU;
    } else {
        // pressed
        button_filters[BUTTON_MIDDLE] = (button_filters[BUTTON_MIDDLE] << 1) | 1U;
    }

    if (SWITCH3) {
        // released
        button_filters[BUTTON_RIGHT] = (button_filters[BUTTON_RIGHT] << 1) & 0xFFFEU;
    } else {
        // pressed
        button_filters[BUTTON_RIGHT] = (button_filters[BUTTON_RIGHT] << 1) | 1U;
    }

    process_buttons();
}

int buttons_is_pressed(int button) {
    int res = 0;

    if (button >= BUTTON_SIZE) {
        return 0;
    }

    if (button_presses[button] == BUTTON_SHORT_PRESS) {
        res = 1;
        button_presses[button] = BUTTON_RELEASED;
    }

    return res;
}
