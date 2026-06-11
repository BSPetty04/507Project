#include "motor.h"
#include "tim.h"

#define MOTOR_PWM_TIMER      htim1
#define MOTOR_PWM_CHANNEL    TIM_CHANNEL_1
#define MOTOR_PWM_MAX        999

static bool motor_running = false;

void Motor_Init(void)
{
    HAL_TIM_PWM_Start(&MOTOR_PWM_TIMER, MOTOR_PWM_CHANNEL);
    Motor_Stop();
}

void Motor_SetForward(void)
{
    // Adjust these if your motor driver direction logic is different.
    HAL_GPIO_WritePin(DIR1_GPIO_Port, DIR1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(DIR2_GPIO_Port, DIR2_Pin, GPIO_PIN_RESET);
}

void Motor_SetDuty(uint16_t duty)
{
    if (duty > MOTOR_PWM_MAX)
    {
        duty = MOTOR_PWM_MAX;
    }

    __HAL_TIM_SET_COMPARE(&MOTOR_PWM_TIMER, MOTOR_PWM_CHANNEL, duty);

    motor_running = duty > 0;
}

void Motor_Stop(void)
{
    __HAL_TIM_SET_COMPARE(&MOTOR_PWM_TIMER, MOTOR_PWM_CHANNEL, 0);

    // Optional brake/coast behavior.
    // This setup makes both direction pins low.
    HAL_GPIO_WritePin(DIR1_GPIO_Port, DIR1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DIR2_GPIO_Port, DIR2_Pin, GPIO_PIN_RESET);

    motor_running = false;
}

bool Motor_IsRunning(void)
{
    return motor_running;
}
