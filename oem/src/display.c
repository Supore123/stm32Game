#include "display.h"
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>

//
// External I2C Handle defined in main.c
//
extern I2C_HandleTypeDef hi2c1;

//
// Global Framebuffer
//
static uint8_t framebuffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];

// ==================== OLED Functions ====================

//
// Sends a single command byte to the OLED via I2C
//
static void OLED_WriteCmd(uint8_t cmd)
{
    HAL_I2C_Mem_Write(&hi2c1, SSD1306_I2C_ADDR, SSD1306_CMD_MODE,
                      I2C_MEMADD_SIZE_8BIT, &cmd, 1, HAL_MAX_DELAY);
}

//
// Initializes the SSD1306 OLED with defined configuration
//
void OLED_Init(void)
{
    vTaskDelay(pdMS_TO_TICKS(100)); // Wait for screen to stabilize

    OLED_WriteCmd(CMD_DISPLAY_OFF);

    OLED_WriteCmd(CMD_SET_DISPLAY_CLK_DIV);
    OLED_WriteCmd(CONF_CLK_DIV_DEFAULT);

    OLED_WriteCmd(CMD_SET_MULTIPLEX);
    OLED_WriteCmd(CONF_MUX_RATIO_64);

    OLED_WriteCmd(CMD_SET_DISPLAY_OFFSET);
    OLED_WriteCmd(CONF_OFFSET_NONE);

    OLED_WriteCmd(CMD_SET_START_LINE | CONF_START_LINE_0);

    OLED_WriteCmd(CMD_CHARGE_PUMP);
    OLED_WriteCmd(CONF_CHARGE_PUMP_EN);

    OLED_WriteCmd(CMD_SET_MEMORY_MODE);
    OLED_WriteCmd(CONF_MEM_MODE_HORIZ);

    OLED_WriteCmd(CMD_SET_SEG_REMAP_1);
    OLED_WriteCmd(CMD_SET_COM_SCAN_DEC);

    OLED_WriteCmd(CMD_SET_COM_PINS);
    OLED_WriteCmd(CONF_COM_PINS_ALT);

    OLED_WriteCmd(CMD_SET_CONTRAST);
    OLED_WriteCmd(CONF_CONTRAST_HIGH);

    OLED_WriteCmd(CMD_SET_PRECHARGE);
    OLED_WriteCmd(CONF_PRECHARGE_DEFAULT);

    OLED_WriteCmd(CMD_SET_VCOMH);
    OLED_WriteCmd(CONF_VCOMH_DEFAULT);

    OLED_WriteCmd(CMD_DISPLAY_ALL_ON);
    OLED_WriteCmd(CMD_DISPLAY_NORMAL);
    OLED_WriteCmd(CMD_DISPLAY_ON);
}

//
// Sends the entire framebuffer to the OLED memory
//
void OLED_Update(void)
{
    for (uint8_t page = 0; page < SSD1306_PAGES; page++)
    {
        // Set the cursor to the start of the current page
        OLED_WriteCmd(CMD_SET_PAGE_START + page);
        OLED_WriteCmd(CMD_SET_COLUMN_LOWER);
        OLED_WriteCmd(CMD_SET_COLUMN_UPPER);

        // Write the page data
        HAL_I2C_Mem_Write(&hi2c1, SSD1306_I2C_ADDR, SSD1306_DATA_MODE,
                          I2C_MEMADD_SIZE_8BIT,
                          &framebuffer[page * SSD1306_WIDTH],
                          SSD1306_WIDTH, HAL_MAX_DELAY);
    }
}

// ==================== Drawing Functions ====================

//
// Clears the local framebuffer array (sets all pixels to 0)
//
void ClearScreen(void)
{
    memset(framebuffer, 0, sizeof(framebuffer));
}

//
// Sets or clears a specific pixel in the framebuffer
//
void SetPixel(int x, int y, uint8_t color)
{
    if (x < 0 || x >= SSD1306_WIDTH || y < 0 || y >= SSD1306_HEIGHT) return;

    if (color) {
        framebuffer[x + (y / 8) * SSD1306_WIDTH] |= (1 << (y % 8));
    } else {
        framebuffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
    }
}

//
// Draws a vertical line with a specific pattern (for wall texturing)
//
void DrawVLine(int x, int y1, int y2, uint8_t pattern)
{
    if (y1 > y2) { int tmp = y1; y1 = y2; y2 = tmp; }

    for (int y = y1; y <= y2; y++)
    {
        SetPixel(x, y, (y + x) % pattern < pattern / 2);
    }
}

//
// Draws a single character using a tiny 3x5 font (Numbers 0-5 only)
//
void DrawChar(int x, int y, char c)
{
    static const uint8_t font[][5] =
    {
        {0x7,0x5,0x5,0x5,0x7}, // 0
        {0x2,0x6,0x2,0x2,0x7}, // 1
        {0x7,0x1,0x7,0x4,0x7}, // 2
        {0x7,0x1,0x7,0x1,0x7}, // 3
        {0x5,0x5,0x7,0x1,0x1}, // 4
        {0x7,0x4,0x7,0x1,0x7}, // 5
    };

    if (c >= '0' && c <= '5') {
        const uint8_t *glyph = font[c - '0'];
        for (int dy = 0; dy < 5; dy++) {
            for (int dx = 0; dx < 3; dx++) {
                if (glyph[dy] & (1 << (2 - dx))) {
                    SetPixel(x + dx, y + dy, 1);
                }
            }
        }
    }
}

//
// Draws an integer number to the screen
//
void DrawNumber(int x, int y, int num)
{
    char buf[8];
    int i = 0;
    if (num == 0) buf[i++] = '0';

    while (num > 0 && i < 7) {
        buf[i++] = '0' + (num % 10);
        num /= 10;
    }
    for (int j = i - 1; j >= 0; j--) {
        DrawChar(x, y, buf[j]);
        x += 4;
    }
}
