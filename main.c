/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

typedef enum
{
  SYS_STOPPED_LOCKED = 0,
  SYS_STOPPED_UNLOCKED,
  SYS_RUNNING,
  SYS_FAULT
} SystemState_t;

typedef enum
{
  BTN_START_STOP = 0,
  BTN_LOCK_UNLOCK,
  BTN_SPEED_TOGGLE,
  BTN_COUNT
} ButtonId_t;

typedef struct
{
  GPIO_TypeDef *port;
  uint16_t pin;
  GPIO_PinState stable_state;
  GPIO_PinState last_raw_state;
  uint32_t last_change_ms;
  bool pressed_event;
} Button_t;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define PWM_MAX_DUTY                 999

#define BUTTON_DEBOUNCE_MS           40
#define DISPLAY_UPDATE_MS            250
#define ENCODER_SAMPLE_MS            100

// Change these later when you know your encoder specs.
// If your encoder is 600 PPR and timer counts x4, use 2400.
#define MOTOR_ENCODER_COUNTS_PER_REV 1024.0f
#define SHAFT_ENCODER_COUNTS_PER_REV 1024.0f

// Assumptions:
// Buttons active-low: pressed = GPIO_PIN_RESET
// Door switch active-low: closed = GPIO_PIN_RESET
// ACT_FAULT active-low: fault = GPIO_PIN_RESET
#define BUTTON_PRESSED_STATE         GPIO_PIN_RESET
#define DOOR_CLOSED_STATE            GPIO_PIN_RESET
#define ACT_FAULT_ACTIVE_STATE       GPIO_PIN_RESET

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
SPI_HandleTypeDef hspi1;
TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

/* USER CODE BEGIN PV */

static SystemState_t system_state = SYS_STOPPED_LOCKED;

static const uint16_t speed_rpm_options[] = {50, 100, 150, 200, 250};
static const uint16_t pwm_duty_options[]  = {200, 350, 500, 650, 800};

static const uint8_t num_speed_options = sizeof(speed_rpm_options) / sizeof(speed_rpm_options[0]);
static uint8_t selected_speed_index = 0;

static Button_t buttons[BTN_COUNT];

static int16_t last_motor_count = 0;
static int16_t last_shaft_count = 0;
static float motor_rpm = 0.0f;
static float shaft_rpm = 0.0f;
static uint32_t last_encoder_sample_ms = 0;

static uint32_t last_display_update_ms = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);

/* USER CODE BEGIN PFP */

static void App_Init(void);
static void App_Update(void);

static void Buttons_Init(void);
static void Buttons_Update(void);
static bool Button_WasPressed(ButtonId_t button);

static bool Door_IsClosed(void);
static bool Actuator_HasFault(void);

static void Motor_Init(void);
static void Motor_Stop(void);
static void Motor_StartSelectedSpeed(void);
static void Motor_SetDuty(uint16_t duty);

static void Actuator_Init(void);
static void Actuator_Stop(void);
static void Actuator_Lock(void);
static void Actuator_Unlock(void);

static void Encoders_Init(void);
static void Encoders_Update(void);

static void Display_Update(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_SPI1_Init();
  MX_TIM1_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_USB_DEVICE_Init();

  /* USER CODE BEGIN 2 */
  App_Init();
  /* USER CODE END 2 */

  while (1)
  {
    /* USER CODE BEGIN 3 */
    App_Update();
    HAL_Delay(5);
    /* USER CODE END 3 */
  }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                              | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{
  ADC_ChannelConfTypeDef sConfig = {0};

  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;

  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_144CYCLES;

  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;

  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 95;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 999;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;

  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;

  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;

  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }

  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;

  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }

  HAL_TIM_MspPostInit(&htim1);
}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{
  TIM_Encoder_InitTypeDef sConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

  sConfig.EncoderMode = TIM_ENCODERMODE_TI12;
  sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC1Filter = 0;
  sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC2Filter = 0;

  if (HAL_TIM_Encoder_Init(&htim3, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;

  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{
  TIM_Encoder_InitTypeDef sConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 0;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 65535;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

  sConfig.EncoderMode = TIM_ENCODERMODE_TI12;
  sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC1Filter = 0;
  sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC2Filter = 0;

  if (HAL_TIM_Encoder_Init(&htim4, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;

  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  HAL_GPIO_WritePin(GPIOA, ACT_EN_Pin | ACT_DIR_Pin | DIR1_Pin | DIR2_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOA, TFT_CS_Pin | TFT_RST_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(ACT_SLEEP_GPIO_Port, ACT_SLEEP_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(TFT_DC_GPIO_Port, TFT_DC_Pin, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin = BTN1_Pin | BTN2_Pin | BTN3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = ACT_EN_Pin | ACT_DIR_Pin | TFT_CS_Pin | DIR1_Pin
                      | DIR2_Pin | TFT_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = SW_NO_Pin | SHAFT_ENC_Z_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = ACT_SLEEP_Pin | TFT_DC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = ACT_FAULT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(ACT_FAULT_GPIO_Port, &GPIO_InitStruct);

  HAL_NVIC_SetPriority(EXTI0_IRQn, 6, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 6, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
}

/* USER CODE BEGIN 4 */

static void App_Init(void)
{
  Buttons_Init();
  Motor_Init();
  Actuator_Init();
  Encoders_Init();

  system_state = SYS_STOPPED_LOCKED;
  last_display_update_ms = HAL_GetTick();
}

static void App_Update(void)
{
  Buttons_Update();
  Encoders_Update();

  bool door_closed = Door_IsClosed();
  bool fault = Actuator_HasFault();

  if (fault)
  {
    Motor_Stop();
    Actuator_Stop();
    system_state = SYS_FAULT;
  }

  if (!door_closed && system_state == SYS_RUNNING)
  {
    Motor_Stop();
    system_state = SYS_STOPPED_LOCKED;
  }

  if (Button_WasPressed(BTN_SPEED_TOGGLE))
  {
    if (system_state != SYS_RUNNING)
    {
      selected_speed_index++;

      if (selected_speed_index >= num_speed_options)
      {
        selected_speed_index = 0;
      }
    }
  }

  if (Button_WasPressed(BTN_START_STOP))
  {
    if (system_state == SYS_RUNNING)
    {
      Motor_Stop();
      system_state = SYS_STOPPED_LOCKED;
    }
    else if (system_state == SYS_STOPPED_LOCKED || system_state == SYS_STOPPED_UNLOCKED)
    {
      if (door_closed && !fault)
      {
        Motor_StartSelectedSpeed();
        system_state = SYS_RUNNING;
      }
    }
    else if (system_state == SYS_FAULT)
    {
      if (!fault)
      {
        system_state = SYS_STOPPED_LOCKED;
      }
    }
  }

  if (Button_WasPressed(BTN_LOCK_UNLOCK))
  {
    // Safety: actuator cannot lock/unlock while motor is running.
    if (system_state != SYS_RUNNING && system_state != SYS_FAULT)
    {
      if (system_state == SYS_STOPPED_LOCKED)
      {
        Actuator_Unlock();
        system_state = SYS_STOPPED_UNLOCKED;
      }
      else
      {
        Actuator_Lock();
        system_state = SYS_STOPPED_LOCKED;
      }

      // Simple actuator pulse.
      // Increase this if the actuator needs more time.
      HAL_Delay(250);
      Actuator_Stop();
    }
  }

  Display_Update();
}

static void Buttons_Init(void)
{
  buttons[BTN_START_STOP].port = BTN1_GPIO_Port;
  buttons[BTN_START_STOP].pin = BTN1_Pin;

  buttons[BTN_LOCK_UNLOCK].port = BTN2_GPIO_Port;
  buttons[BTN_LOCK_UNLOCK].pin = BTN2_Pin;

  buttons[BTN_SPEED_TOGGLE].port = BTN3_GPIO_Port;
  buttons[BTN_SPEED_TOGGLE].pin = BTN3_Pin;

  for (uint8_t i = 0; i < BTN_COUNT; i++)
  {
    GPIO_PinState raw = HAL_GPIO_ReadPin(buttons[i].port, buttons[i].pin);

    buttons[i].stable_state = raw;
    buttons[i].last_raw_state = raw;
    buttons[i].last_change_ms = HAL_GetTick();
    buttons[i].pressed_event = false;
  }
}

static void Buttons_Update(void)
{
  uint32_t now = HAL_GetTick();

  for (uint8_t i = 0; i < BTN_COUNT; i++)
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

        if (old_state != BUTTON_PRESSED_STATE && raw == BUTTON_PRESSED_STATE)
        {
          buttons[i].pressed_event = true;
        }
      }
    }
  }
}

static bool Button_WasPressed(ButtonId_t button)
{
  if (button >= BTN_COUNT)
  {
    return false;
  }

  bool was_pressed = buttons[button].pressed_event;
  buttons[button].pressed_event = false;

  return was_pressed;
}

static bool Door_IsClosed(void)
{
  return HAL_GPIO_ReadPin(SW_NO_GPIO_Port, SW_NO_Pin) == DOOR_CLOSED_STATE;
}

static bool Actuator_HasFault(void)
{
  return HAL_GPIO_ReadPin(ACT_FAULT_GPIO_Port, ACT_FAULT_Pin) == ACT_FAULT_ACTIVE_STATE;
}

static void Motor_Init(void)
{
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  Motor_Stop();
}

static void Motor_SetDuty(uint16_t duty)
{
  if (duty > PWM_MAX_DUTY)
  {
    duty = PWM_MAX_DUTY;
  }

  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, duty);
}

static void Motor_StartSelectedSpeed(void)
{
  // Direction assumption: DIR1 high, DIR2 low = forward.
  HAL_GPIO_WritePin(DIR1_GPIO_Port, DIR1_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(DIR2_GPIO_Port, DIR2_Pin, GPIO_PIN_RESET);

  Motor_SetDuty(pwm_duty_options[selected_speed_index]);
}

static void Motor_Stop(void)
{
  Motor_SetDuty(0);

  // Coast/stop assumption: both direction pins low.
  HAL_GPIO_WritePin(DIR1_GPIO_Port, DIR1_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(DIR2_GPIO_Port, DIR2_Pin, GPIO_PIN_RESET);
}

static void Actuator_Init(void)
{
  // Assumption: ACT_SLEEP high = actuator driver awake.
  HAL_GPIO_WritePin(ACT_SLEEP_GPIO_Port, ACT_SLEEP_Pin, GPIO_PIN_SET);
  Actuator_Stop();
}

static void Actuator_Stop(void)
{
  HAL_GPIO_WritePin(ACT_EN_GPIO_Port, ACT_EN_Pin, GPIO_PIN_RESET);
}

static void Actuator_Lock(void)
{
  if (Actuator_HasFault())
  {
    Actuator_Stop();
    system_state = SYS_FAULT;
    return;
  }

  // Assumption: ACT_DIR low = lock.
  HAL_GPIO_WritePin(ACT_DIR_GPIO_Port, ACT_DIR_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(ACT_EN_GPIO_Port, ACT_EN_Pin, GPIO_PIN_SET);
}

static void Actuator_Unlock(void)
{
  if (Actuator_HasFault())
  {
    Actuator_Stop();
    system_state = SYS_FAULT;
    return;
  }

  // Assumption: ACT_DIR high = unlock.
  HAL_GPIO_WritePin(ACT_DIR_GPIO_Port, ACT_DIR_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(ACT_EN_GPIO_Port, ACT_EN_Pin, GPIO_PIN_SET);
}

static void Encoders_Init(void)
{
  HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
  HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);

  __HAL_TIM_SET_COUNTER(&htim3, 0);
  __HAL_TIM_SET_COUNTER(&htim4, 0);

  last_motor_count = 0;
  last_shaft_count = 0;
  last_encoder_sample_ms = HAL_GetTick();
}

static void Encoders_Update(void)
{
  uint32_t now = HAL_GetTick();

  if ((now - last_encoder_sample_ms) < ENCODER_SAMPLE_MS)
  {
    return;
  }

  float dt_minutes = (float)(now - last_encoder_sample_ms) / 60000.0f;

  int16_t motor_count = (int16_t)__HAL_TIM_GET_COUNTER(&htim3);
  int16_t shaft_count = (int16_t)__HAL_TIM_GET_COUNTER(&htim4);

  int16_t motor_delta = motor_count - last_motor_count;
  int16_t shaft_delta = shaft_count - last_shaft_count;

  motor_rpm = ((float)motor_delta / MOTOR_ENCODER_COUNTS_PER_REV) / dt_minutes;
  shaft_rpm = ((float)shaft_delta / SHAFT_ENCODER_COUNTS_PER_REV) / dt_minutes;

  last_motor_count = motor_count;
  last_shaft_count = shaft_count;
  last_encoder_sample_ms = now;
}

static void Display_Update(void)
{
  uint32_t now = HAL_GetTick();

  if ((now - last_display_update_ms) < DISPLAY_UPDATE_MS)
  {
    return;
  }

  last_display_update_ms = now;

  // Placeholder until TFT driver is added.
  // For now, just keep TFT backlight on.
  HAL_GPIO_WritePin(TFT_LITE_GPIO_Port, TFT_LITE_Pin, GPIO_PIN_SET);

  // Variables available for display later:
  // speed_rpm_options[selected_speed_index]
  // motor_rpm
  // shaft_rpm
  // system_state
  // Door_IsClosed()
  // Actuator_HasFault()

  (void)motor_rpm;
  (void)shaft_rpm;
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  __disable_irq();

  while (1)
  {
  }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
  (void)file;
  (void)line;
}
#endif /* USE_FULL_ASSERT */
