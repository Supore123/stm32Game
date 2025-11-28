#include "main.h"
#include "i2c.h"

//
// External handle declaration as requested
//
extern I2C_HandleTypeDef hi2c1;

//
// Initialize the I2C1 peripheral and GPIO pins using HAL
//
void I2C_Init(void)
{
    // 1. Enable Clocks
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_I2C1_CLK_ENABLE();

    // 2. Configure GPIO (PB8=SCL, PB9=SDA)
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;       // Open Drain for I2C
    GPIO_InitStruct.Pull = GPIO_PULLUP;           // Pull-up resistors
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;    // Alternate Function 4
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // 3. Configure I2C1 Peripheral
    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 100000;               // 100 kHz
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    HAL_I2C_Init(&hi2c1);
}

//
// Transmit data to a device (Replaces Start, WriteAddr, Write, Stop)
//
void I2C_Write(uint8_t devAddr, uint8_t *data, uint16_t size)
{
    // HAL automatically handles START, ADDRESS, DATA, and STOP conditions
    HAL_I2C_Master_Transmit(&hi2c1, devAddr, data, size, HAL_MAX_DELAY);
}

//
// Transmit a single command byte (Useful wrapper for OLEDs)
//
void I2C_WriteByte(uint8_t devAddr, uint8_t byte)
{
    HAL_I2C_Master_Transmit(&hi2c1, devAddr, &byte, 1, HAL_MAX_DELAY);
}
