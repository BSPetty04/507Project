/**
 * @file app.c
 * @brief Top-level gearbox application state machine and PI speed controller.
 *
 * This module owns the main application logic. It combines safety interlocks,
 * user button events, actuator lock state, encoder feedback, and PI output shaft
 * speed control. The motor is only allowed to run when the door switch indicates
 * that the enclosure is closed. The actuator is blocked while the gearbox is
 * running.
 */
#include "app.h"
#include "motor.h"
#include "actuator.h"
#include "buttons.h"
#include "encoders.h"
#include "ui.h"
#include <math.h>

/** High-level application states. */
typedef enum
{
    APP_STOPPED_LOCKED = 0,     /**< Gearbox stopped and lock state recorded as locked. */
    APP_STOPPED_UNLOCKED,       /**< Gearbox stopped and lock state recorded as unlocked. */
    APP_RUNNING,                /**< Motor enabled and PI controller actively regulating output shaft speed. */
    APP_FAULT                   /**< Fault state. Motor and actuator are disabled. */
} AppState_t;

static AppState_t app_state = APP_STOPPED_LOCKED;

/** User-selectable closed-loop output shaft speed targets in RPM. */
static const uint16_t speed_rpm_options[] = {50, 100, 150, 200, 250};
#define NUM_SPEED_OPTIONS  (sizeof(speed_rpm_options) / sizeof(speed_rpm_options[0]))

/** PI control update period. */
#define CONTROL_UPDATE_MS  50U

/** PWM duty limits for TIM1 period of 999 counts. */
#define PWM_MIN_DUTY      0U
#define PWM_MAX_DUTY      999U

/** Initial PI gains. These should be tuned on the physical gearbox. */
#define SPEED_KP          2.0f
#define SPEED_KI          0.5f

/** Anti-windup limits for the integral term. */
#define INTEGRAL_MIN      -500.0f
#define INTEGRAL_MAX       500.0f

/** Small starting duty used to overcome static friction before the PI loop takes over. */
#define STARTUP_DUTY      150U

static uint8_t selected_speed_index = 0;
static float speed_integral = 0.0f;
static uint32_t last_control_update_ms = 0;

/**
 * @brief Read the door/reed switch safety input.
 * @return true if the enclosure is detected closed.
 *
 * The current prototype assumes the door switch is active-low. If the final
 * wiring changes, this is the only function that needs to be updated.
 */
static bool Door_IsClosed(void)
{
    return HAL_GPIO_ReadPin(SW_NO_GPIO_Port, SW_NO_Pin) == GPIO_PIN_RESET;
}

/** Clamp a floating-point controller output to valid PWM compare counts. */
static uint16_t ClampDuty(float duty)
{
    if (duty < (float)PWM_MIN_DUTY)
    {
        return PWM_MIN_DUTY;
    }
    if (duty > (float)PWM_MAX_DUTY)
    {
        return PWM_MAX_DUTY;
    }
    return (uint16_t)duty;
}

/** Reset PI memory when starting, stopping, or faulting. */
static void SpeedController_Reset(void)
{
    speed_integral = 0.0f;
    last_control_update_ms = HAL_GetTick();
}

/**
 * @brief Closed-loop PI controller for output shaft speed.
 *
 * The selected speed preset is the target RPM. The measured feedback is the
 * output shaft RPM reported by Encoders_GetShaftRPM(). The PI controller updates
 * motor PWM duty cycle to reduce speed error. The integral term is clamped to
 * reduce windup when the motor saturates or the system is stopped.
 */
static void SpeedController_Update(void)
{
    uint32_t now = HAL_GetTick();

    if ((now - last_control_update_ms) < CONTROL_UPDATE_MS)
    {
        return;
    }

    float dt_seconds = (float)(now - last_control_update_ms) / 1000.0f;
    last_control_update_ms = now;

    float target_rpm = (float)speed_rpm_options[selected_speed_index];
    float measured_rpm = Encoders_GetShaftRPM();

    /* Use magnitude so an inverted encoder direction does not reverse the controller. */
    if (measured_rpm < 0.0f)
    {
        measured_rpm = -measured_rpm;
    }

    float error = target_rpm - measured_rpm;
    speed_integral += error * dt_seconds;

    if (speed_integral > INTEGRAL_MAX)
    {
        speed_integral = INTEGRAL_MAX;
    }
    else if (speed_integral < INTEGRAL_MIN)
    {
        speed_integral = INTEGRAL_MIN;
    }

    float duty = (SPEED_KP * error) + (SPEED_KI * speed_integral);
    Motor_SetDuty(ClampDuty(duty));
}

/** Start the motor only if all safety conditions permit operation. */
static void StartMotorIfSafe(void)
{
    if (!Door_IsClosed())
    {
        Motor_Stop();
        return;
    }

    if (Actuator_HasFault())
    {
        Motor_Stop();
        app_state = APP_FAULT;
        return;
    }

    SpeedController_Reset();
    Motor_SetForward();
    Motor_SetDuty(STARTUP_DUTY);
    app_state = APP_RUNNING;
}

/** Stop the motor and return to the correct stopped state. */
static void StopMotor(void)
{
    Motor_Stop();
    SpeedController_Reset();

    if (Actuator_GetState() == ACTUATOR_LOCKED)
    {
        app_state = APP_STOPPED_LOCKED;
    }
    else
    {
        app_state = APP_STOPPED_UNLOCKED;
    }
}

void APP_Init(void)
{
    Motor_Init();
    Actuator_Init();
    Buttons_Init();
    Encoders_Init();
    UI_Init();

    SpeedController_Reset();
    app_state = APP_STOPPED_LOCKED;
}

void APP_Update(void)
{
    Buttons_Update();
    Encoders_Update();

    bool door_closed = Door_IsClosed();
    bool fault = Actuator_HasFault();

    if (fault)
    {
        Motor_Stop();
        Actuator_Stop();
        SpeedController_Reset();
        app_state = APP_FAULT;
    }

    if (!door_closed && app_state == APP_RUNNING)
    {
        Motor_Stop();
        SpeedController_Reset();
        app_state = APP_STOPPED_LOCKED;
    }

    if (Buttons_WasPressed(BUTTON_SPEED_TOGGLE))
    {
        if (app_state != APP_RUNNING)
        {
            selected_speed_index++;
            if (selected_speed_index >= NUM_SPEED_OPTIONS)
            {
                selected_speed_index = 0;
            }
        }
    }

    if (Buttons_WasPressed(BUTTON_START_STOP))
    {
        if (app_state == APP_RUNNING)
        {
            StopMotor();
        }
        else if (app_state == APP_STOPPED_LOCKED || app_state == APP_STOPPED_UNLOCKED)
        {
            StartMotorIfSafe();
        }
        else if (app_state == APP_FAULT)
        {
            if (!fault)
            {
                app_state = APP_STOPPED_LOCKED;
            }
        }
    }

    if (Buttons_WasPressed(BUTTON_LOCK_UNLOCK))
    {
        /* Safety rule: cannot lock/unlock while motor is running. */
        if (app_state != APP_RUNNING && app_state != APP_FAULT)
        {
            if (Actuator_GetState() == ACTUATOR_LOCKED)
            {
                Actuator_Unlock();
                app_state = APP_STOPPED_UNLOCKED;
            }
            else
            {
                Actuator_Lock();
                app_state = APP_STOPPED_LOCKED;
            }

            /* Prototype behavior: command actuator briefly, then disable output. */
            Actuator_Stop();
        }
    }

    if (app_state == APP_RUNNING)
    {
        SpeedController_Update();
    }

    UI_Update(
        Encoders_GetMotorRPM(),
        Encoders_GetShaftRPM(),
        speed_rpm_options[selected_speed_index],
        app_state == APP_RUNNING,
        Actuator_GetState() == ACTUATOR_LOCKED,
        door_closed,
        fault
    );
}
