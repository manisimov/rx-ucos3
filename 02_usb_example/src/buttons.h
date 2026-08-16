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

#endif // BUTTONS_H
