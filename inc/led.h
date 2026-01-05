#ifndef LED_H
#define LED_H

#include <stdbool.h>

void led_init(void);

// LED RGB rosso (solo canale rosso)
void led_red_on(void);
void led_red_off(void);
void led_red_toggle(void);

#endif
