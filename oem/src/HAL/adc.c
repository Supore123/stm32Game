/* oem/src/HAL/adc.c */
#include "adc.h"
#include "main.h"

// External Handle from main.c
extern ADC_HandleTypeDef hadc1;

// --- DMA Buffer ---
// volatile tells the compiler that hardware modifies this memory.
// uint16_t due to half word use
volatile uint16_t adc_dma_buffer[2];

void ADC_Joystick_Init(void)
{
    // Initialize Button GPIO (Standard Input)
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = JOY_PIN_BUTTON;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(JOY_PORT_BUTTON, &GPIO_InitStruct);

    // The ADC will now run forever in the background, updating 'adc_dma_buffer'.
    // We do NOT need a Mutex anymore because we are only READING from this buffer.
    if (HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_dma_buffer, 2) != HAL_OK)
    {
        Error_Handler();
    }
}

// DMA Getters
uint32_t ADC_GetX(void)
{
    // Returns the latest X value instantly from RAM
    return adc_dma_buffer[0];
}

uint32_t ADC_GetY(void)
{
    // Returns the latest Y value instantly from RAM
    return adc_dma_buffer[1];
}

uint8_t ADC_ReadButton(void)
{
    // Returns 1 if Pressed (Active Low), 0 if Released
    return (HAL_GPIO_ReadPin(JOY_PORT_BUTTON, JOY_PIN_BUTTON) == GPIO_PIN_RESET) ? 1 : 0;
}
