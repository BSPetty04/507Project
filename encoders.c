#include "encoders.h"
#include "tim.h"

#define ENCODER_SAMPLE_MS       100

// Update these once you know your encoder counts per revolution.
// For quadrature x4 counting, effective counts may be PPR * 4.
#define MOTOR_ENCODER_COUNTS_PER_REV  1024.0f
#define SHAFT_ENCODER_COUNTS_PER_REV  1024.0f

static int16_t last_motor_count = 0;
static int16_t last_shaft_count = 0;

static float motor_rpm = 0.0f;
static float shaft_rpm = 0.0f;

static uint32_t last_sample_ms = 0;

void Encoders_Init(void)
{
    HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
    HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);

    __HAL_TIM_SET_COUNTER(&htim3, 0);
    __HAL_TIM_SET_COUNTER(&htim4, 0);

    last_motor_count = 0;
    last_shaft_count = 0;
    last_sample_ms = HAL_GetTick();
}

void Encoders_Update(void)
{
    uint32_t now = HAL_GetTick();

    if ((now - last_sample_ms) < ENCODER_SAMPLE_MS)
    {
        return;
    }

    float dt_minutes = (float)(now - last_sample_ms) / 60000.0f;

    int16_t motor_count = (int16_t)__HAL_TIM_GET_COUNTER(&htim3);
    int16_t shaft_count = (int16_t)__HAL_TIM_GET_COUNTER(&htim4);

    int16_t motor_delta = motor_count - last_motor_count;
    int16_t shaft_delta = shaft_count - last_shaft_count;

    motor_rpm = ((float)motor_delta / MOTOR_ENCODER_COUNTS_PER_REV) / dt_minutes;
    shaft_rpm = ((float)shaft_delta / SHAFT_ENCODER_COUNTS_PER_REV) / dt_minutes;

    last_motor_count = motor_count;
    last_shaft_count = shaft_count;
    last_sample_ms = now;
}

float Encoders_GetMotorRPM(void)
{
    return motor_rpm;
}

float Encoders_GetShaftRPM(void)
{
    return shaft_rpm;
}
