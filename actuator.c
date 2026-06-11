#include "actuator.h"

static ActuatorState_t actuator_state = ACTUATOR_LOCKED;

void Actuator_Init(void)
{
    // Assumption: ACT_SLEEP high wakes driver.
    HAL_GPIO_WritePin(ACT_SLEEP_GPIO_Port, ACT_SLEEP_Pin, GPIO_PIN_SET);

    // Disable actuator initially.
    HAL_GPIO_WritePin(ACT_EN_GPIO_Port, ACT_EN_Pin, GPIO_PIN_RESET);

    actuator_state = ACTUATOR_LOCKED;
}

bool Actuator_HasFault(void)
{
    // Assumption: fault active-low.
    return HAL_GPIO_ReadPin(ACT_FAULT_GPIO_Port, ACT_FAULT_Pin) == GPIO_PIN_RESET;
}

void Actuator_Lock(void)
{
    if (Actuator_HasFault())
    {
        Actuator_Stop();
        return;
    }

    // Assumption: ACT_DIR low = lock.
    HAL_GPIO_WritePin(ACT_DIR_GPIO_Port, ACT_DIR_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(ACT_EN_GPIO_Port, ACT_EN_Pin, GPIO_PIN_SET);

    actuator_state = ACTUATOR_LOCKED;
}

void Actuator_Unlock(void)
{
    if (Actuator_HasFault())
    {
        Actuator_Stop();
        return;
    }

    // Assumption: ACT_DIR high = unlock.
    HAL_GPIO_WritePin(ACT_DIR_GPIO_Port, ACT_DIR_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(ACT_EN_GPIO_Port, ACT_EN_Pin, GPIO_PIN_SET);

    actuator_state = ACTUATOR_UNLOCKED;
}

void Actuator_Stop(void)
{
    HAL_GPIO_WritePin(ACT_EN_GPIO_Port, ACT_EN_Pin, GPIO_PIN_RESET);
}

ActuatorState_t Actuator_GetState(void)
{
    return actuator_state;
}
