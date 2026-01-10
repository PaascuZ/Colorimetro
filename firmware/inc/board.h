#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>
#include <stdbool.h>

void board_init(void);

/* --- INT4 / BTNC --- */
void board_int4_btnc_init(void);
bool board_int4_btnc_fired(void);     // true una volta quando arriva l'interrupt
void board_int4_btnc_clear(void);     // clear del flag software

#endif
