#ifndef ENCODERS_H
#define ENCODERS_H

#include "main.h"
#include <stdint.h>

void Encoders_Init(void);
void Encoders_Update(void);

float Encoders_GetMotorRPM(void);
float Encoders_GetShaftRPM(void);

#endif
