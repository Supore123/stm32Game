#include "adc.h"

//
// External ADC Handle defined in main.c
//
extern ADC_HandleTypeDef hadc1;

// ==================== ADC & Joystick Functions ====================

//
// Initializes ADC1 for Joystick Axes and GPIOB for the Button
//
void ADC_Init(void)
{
    // 1. Enable Peripheral Clocks
    __HAL_RCC_ADC1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE(); // For Analog Pins (PA0, PA1)
    __HAL_RCC_GPIOB_CLK_ENABLE(); // For Button Pin (PB3)

    // 2. Configure GPIO for Analog Inputs (PA0, PA1)
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // 3. Configure GPIO for Button Input (PB3)
    // Note: We use Pull-Up because the button connects to GND when pressed
    GPIO_InitStruct.Pin = JOY_BTN_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(JOY_BTN_PORT, &GPIO_InitStruct);

    // 4. Configure ADC Peripheral
    hadc1.Instance = JOY_ADC_INSTANCE;
    hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
    hadc1.Init.Resolution = ADC_RESOLUTION_VAL;
    hadc1.Init.ScanConvMode = DISABLE;             // Single channel at a time
    hadc1.Init.ContinuousConvMode = DISABLE;       // Single shot mode
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = 1;
    hadc1.Init.DMAContinuousRequests = DISABLE;
    hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;

    HAL_ADC_Init(&hadc1);
}

//
// Configures the channel, starts conversion, and returns the value
//
uint16_t ADC_Read(uint8_t channel_index)
{
    ADC_ChannelConfTypeDef sConfig = {0};

    // Common Configuration
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLE_TIME;

    // Map the index (0 or 1) to the actual HAL Channel constant
    if (channel_index == 0) {
        sConfig.Channel = JOY_AXIS_X_CHANNEL;
    } else {
        sConfig.Channel = JOY_AXIS_Y_CHANNEL;
    }

    // Apply Channel Config
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);

    // Start Conversion
    HAL_ADC_Start(&hadc1);

    // Wait for completion
    HAL_ADC_PollForConversion(&hadc1, ADC_TIMEOUT);

    // Get Value
    return HAL_ADC_GetValue(&hadc1);
}

//
// Helper to read the digital button state (0 = Pressed, 1 = Released)
//
uint8_t ADC_ReadButton(void)
{
    // Returns GPIO_PIN_RESET (0) if pressed, GPIO_PIN_SET (1) if released
    return HAL_GPIO_ReadPin(JOY_BTN_PORT, JOY_BTN_PIN);
}
