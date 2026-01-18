#include "adc.h"
#include "main.h"

// External ADC Handle defined in main.c
extern ADC_HandleTypeDef hadc1;

// CMSIS-RTOS v2 Mutex to protect the shared ADC peripheral
osMutexId_t adc_mutex = NULL;

/**
 * @brief Initializes ADC1 and GPIOs for Joystick.
 */
void ADC_Joystick_Init(void)
{
    // 1. Create the Mutex (CMSIS-RTOS v2)
    if (adc_mutex == NULL)
    {
        adc_mutex = osMutexNew(NULL);
    }

    // 2. Enable Peripheral Clocks
    __HAL_RCC_ADC1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    // 3. Configure GPIO for Analog Inputs (PA0, PA1)
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = JOY_PIN_X | JOY_PIN_Y;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(JOY_PORT_ANALOG, &GPIO_InitStruct);

    // 4. Configure GPIO for Button Input (PA10)
    GPIO_InitStruct.Pin = JOY_PIN_BUTTON;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(JOY_PORT_BUTTON, &GPIO_InitStruct);

    // 5. Configure ADC Peripheral Settings
    hadc1.Instance = JOY_ADC_INSTANCE;
    hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
    hadc1.Init.Resolution = ADC_RESOLUTION_VAL;
    hadc1.Init.ScanConvMode = DISABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = 1;
    hadc1.Init.DMAContinuousRequests = DISABLE;
    hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;

    HAL_ADC_Init(&hadc1);
}

/**
 * @brief Thread-safe ADC read using CMSIS-RTOS v2 Mutex.
 * Demonstrates "Smooth and responsive gameplay" by preventing data corruption.
 */
uint16_t ADC_Read_Locked(uint32_t channel)
{
    ADC_ChannelConfTypeDef sConfig = {0};
    uint16_t result = 0;

    // Check if the mutex exists (avoids crashes if task starts before init)
    if (adc_mutex != NULL)
    {
        // CMSIS-RTOS v2: Attempt to acquire Mutex
        if (osMutexAcquire(adc_mutex, ADC_TIMEOUT_MS) == osOK)
        {
            // Configure the selected channel
            sConfig.Channel = channel;
            sConfig.Rank = 1;
            sConfig.SamplingTime = ADC_SAMPLE_TIME;
            HAL_ADC_ConfigChannel(&hadc1, &sConfig);

            // Perform Conversion
            HAL_ADC_Start(&hadc1);
            if (HAL_ADC_PollForConversion(&hadc1, ADC_TIMEOUT_MS) == HAL_OK)
            {
                result = HAL_ADC_GetValue(&hadc1);
            }
            HAL_ADC_Stop(&hadc1);

            // Release Mutex
            osMutexRelease(adc_mutex);
        }
    }
    return result;
}

/**
 * @brief Helper to read the digital button state.
 * Returns 1 if Pressed (Logic Low), 0 if Released (Logic High).
 */
uint8_t ADC_ReadButton(void)
{
    // Active-Low: RESET means button is physically pressed
    if (HAL_GPIO_ReadPin(JOY_PORT_BUTTON, JOY_PIN_BUTTON) == GPIO_PIN_RESET)
    {
        return 1;
    }
    return 0;
}
