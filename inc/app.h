#ifndef APP_H
#define APP_H

#include <stdint.h>
#include <stdbool.h>

// Inizializza tutta l'applicazione (driver + stato applicativo)
void app_init(void);

// Superloop task: da chiamare continuamente in while(1)
void app_task(void);

#endif // APP_H
