#ifndef __ADC_H
#define __ADC_H

#include "stm32f4xx_hal.h"
#include "cmsis_os2.h" // CMSIS-RTOS v2 for best practices

// --- Joystick Hardware Pin Definitions ---
#define JOY_PORT_ANALOG          GPIOA
#define JOY_PIN_X                GPIO_PIN_0
#define JOY_PIN_Y                GPIO_PIN_1

#define JOY_PORT_BUTTON          GPIOA
#define JOY_PIN_BUTTON           GPIO_PIN_10

// --- ADC Configuration Definitions ---
#define JOY_ADC_INSTANCE         ADC1
#define JOY_CHANNEL_X            ADC_CHANNEL_0
#define JOY_CHANNEL_Y            ADC_CHANNEL_1

// --- ADC Performance Constants ---
#define ADC_RESOLUTION_VAL       ADC_RESOLUTION_12B
#define ADC_SAMPLE_TIME          ADC_SAMPLETIME_480CYCLES
#define ADC_TIMEOUT_MS           10

// --- FreeRTOS Synchronization Handle (CMSIS-RTOS v2) ---
extern osMutexId_t adc_mutex;

// --- Function Prototypes ---
void ADC_Joystick_Init(void);
uint16_t ADC_Read_Locked(uint32_t channel);      // Thread-safe read
uint8_t ADC_ReadButton(void);                    // Digital read

#endif /* __ADC_H */
