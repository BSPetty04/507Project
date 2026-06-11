#include "ui.h"

// Placeholder UI.
// Once you tell me the TFT driver/chip, we will replace this with real display code.

void UI_Init(void)
{
    // Turn backlight on.
    HAL_GPIO_WritePin(TFT_LITE_GPIO_Port, TFT_LITE_Pin, GPIO_PIN_SET);

    // Release display reset.
    HAL_GPIO_WritePin(TFT_RST_GPIO_Port, TFT_RST_Pin, GPIO_PIN_SET);
}

void UI_Update(float motor_rpm, float shaft_rpm, uint16_t selected_speed, bool running, bool locked, bool door_closed, bool fault)
{
    (void)motor_rpm;
    (void)shaft_rpm;
    (void)selected_speed;
    (void)running;
    (void)locked;
    (void)door_closed;
    (void)fault;

    // Display code goes here later.
}
