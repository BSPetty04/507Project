#ifndef MOTOR_H
#define MOTOR_H

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

void Motor_Init(void);
void Motor_Stop(void);
void Motor_SetDuty(uint16_t duty);
void Motor_SetForward(void);
bool Motor_IsRunning(void);

#endif
