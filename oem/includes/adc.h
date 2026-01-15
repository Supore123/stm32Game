#ifndef __ADC_H
#define __ADC_H

#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "semphr.h"

//
// Joystick Hardware Pin Definitions
//
#define JOY_PORT_ANALOG          GPIOA           // Joystick X/Y are on Port A
#define JOY_PIN_X                GPIO_PIN_0      // PA0
#define JOY_PIN_Y                GPIO_PIN_1      // PA1

#define JOY_PORT_BUTTON          GPIOB           // Joystick Switch is on Port B
#define JOY_PIN_BUTTON           GPIO_PIN_3      // PB3

//
// ADC Configuration Definitions
//
#define JOY_ADC_INSTANCE         ADC1
#define JOY_CHANNEL_X            ADC_CHANNEL_0   // Channel 0 corresponds to PA0
#define JOY_CHANNEL_Y            ADC_CHANNEL_1   // Channel 1 corresponds to PA1

//
// ADC Performance Constants
//
#define ADC_RESOLUTION_VAL       ADC_RESOLUTION_12B
#define ADC_SAMPLE_TIME          ADC_SAMPLETIME_480CYCLES // High sampling time for stability
#define ADC_TIMEOUT              10              // Timeout in ms
#define ADC_STABILIZE_DELAY      10              // Delay to let ADC power up

//
// FreeRTOS Synchronization Handle
//
extern SemaphoreHandle_t adc_mutex;

//
// Function Prototypes
//
void ADC_Joystick_Init(void);
uint16_t ADC_Read_Locked(uint32_t channel);      // Thread-safe read
uint8_t ADC_ReadButton(void);                    // Digital read

#endif /* __ADC_H */
