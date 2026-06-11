#include "buttons.h"

#define BUTTON_DEBOUNCE_MS 30

typedef struct
{
    GPIO_TypeDef *port;
    uint16_t pin;
    GPIO_PinState stable_state;
    GPIO_PinState last_raw_state;
    uint32_t last_change_ms;
    bool pressed_event;
} Button_t;

static Button_t buttons[BUTTON_COUNT];

static bool ButtonRawPressed(GPIO_PinState state)
{
    // Assumption: active-low buttons.
    return state == GPIO_PIN_RESET;
}

void Buttons_Init(void)
{
    buttons[BUTTON_START_STOP] = (Button_t){BTN1_GPIO_Port, BTN1_Pin, GPIO_PIN_SET, GPIO_PIN_SET, 0, false};
    buttons[BUTTON_LOCK_UNLOCK] = (Button_t){BTN2_GPIO_Port, BTN2_Pin, GPIO_PIN_SET, GPIO_PIN_SET, 0, false};
    buttons[BUTTON_SPEED_TOGGLE] = (Button_t){BTN3_GPIO_Port, BTN3_Pin, GPIO_PIN_SET, GPIO_PIN_SET, 0, false};
}

void Buttons_Update(void)
{
    uint32_t now = HAL_GetTick();

    for (int i = 0; i < BUTTON_COUNT; i++)
    {
        GPIO_PinState raw = HAL_GPIO_ReadPin(buttons[i].port, buttons[i].pin);

        if (raw != buttons[i].last_raw_state)
        {
            buttons[i].last_raw_state = raw;
            buttons[i].last_change_ms = now;
        }

        if ((now - buttons[i].last_change_ms) >= BUTTON_DEBOUNCE_MS)
        {
            if (raw != buttons[i].stable_state)
            {
                GPIO_PinState old_state = buttons[i].stable_state;
                buttons[i].stable_state = raw;

                if (!ButtonRawPressed(old_state) && ButtonRawPressed(raw))
                {
                    buttons[i].pressed_event = true;
                }
            }
        }
    }
}

bool Buttons_WasPressed(ButtonId_t button)
{
    if (button >= BUTTON_COUNT)
    {
        return false;
    }

    bool event = buttons[button].pressed_event;
    buttons[button].pressed_event = false;
    return event;
}
