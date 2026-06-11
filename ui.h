#ifndef UI_H
#define UI_H

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

void UI_Init(void);
void UI_Update(float motor_rpm, float shaft_rpm, uint16_t selected_speed, bool running, bool locked, bool door_closed, bool fault);

#endif
