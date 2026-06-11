#ifndef ACTUATOR_H
#define ACTUATOR_H

#include "main.h"
#include <stdbool.h>

typedef enum
{
    ACTUATOR_LOCKED = 0,
    ACTUATOR_UNLOCKED
} ActuatorState_t;

void Actuator_Init(void);
void Actuator_Lock(void);
void Actuator_Unlock(void);
void Actuator_Stop(void);
bool Actuator_HasFault(void);
ActuatorState_t Actuator_GetState(void);

#endif
