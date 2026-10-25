#ifndef BUTTONS_H
#define BUTTONS_H

enum {
    BUTTON_LEFT = 0,
    BUTTON_MIDDLE,
    BUTTON_RIGHT,
    BUTTON_SIZE
};

void buttons_init(void);
void buttons_task(void);
//int  buttons_get_state(void);
int buttons_is_pressed(int button);
// Debounced current state: 1 while button is held down. Has shorter filter
// than buttons_is_pressed(), so it can be used as mouse button.
int buttons_is_down(int button);

#endif // BUTTONS_H
