#ifndef ADC_H
#define ADC_H

#include "stm32f4xx_hal.h"

//
// Joystick Analog Definitions
//
#define JOY_AXIS_X_CHANNEL      ADC_CHANNEL_0   // Connected to PA0
#define JOY_AXIS_Y_CHANNEL      ADC_CHANNEL_1   // Connected to PA1
#define JOY_ADC_INSTANCE        ADC1

//
// Joystick Button Definitions (Digital)
//
#define JOY_BTN_PIN             GPIO_PIN_3      // Connected to PB3
#define JOY_BTN_PORT            GPIOB

//
// ADC Configuration Constants
//
#define ADC_SAMPLE_TIME         ADC_SAMPLETIME_480CYCLES
#define ADC_RESOLUTION_VAL      ADC_RESOLUTION_12B
#define ADC_TIMEOUT             10  // ms to wait for conversion

//
// Function Prototypes
//
void ADC_Init(void);
uint16_t ADC_Read(uint8_t channel_index);
uint8_t ADC_ReadButton(void);

#endif // ADC_H
