#ifndef BUTTONS_H
#define BUTTONS_H

#include "main.h"
#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    BUTTON_START_STOP = 0,
    BUTTON_LOCK_UNLOCK,
    BUTTON_SPEED_TOGGLE,
    BUTTON_COUNT
} ButtonId_t;

void Buttons_Init(void);
void Buttons_Update(void);
bool Buttons_WasPressed(ButtonId_t button);

#endif
