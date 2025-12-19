#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

//
// I2C Configuration
//
#define SSD1306_I2C_ADDR        (0x78) // 0x3C << 1
#define SSD1306_CMD_MODE        0x00
#define SSD1306_DATA_MODE       0x40

//
// Screen Dimensions
//
#define SSD1306_WIDTH           128
#define SSD1306_HEIGHT          64
#define SSD1306_PAGES           (SSD1306_HEIGHT / 8)

//
// Fundamental Commands
//
#define CMD_DISPLAY_OFF         0xAE
#define CMD_DISPLAY_ON          0xAF
#define CMD_SET_CONTRAST        0x81
#define CMD_DISPLAY_ALL_ON      0xA4
#define CMD_DISPLAY_NORMAL      0xA6
#define CMD_DISPLAY_INVERT      0xA7

//
// Addressing Setting Commands
//
#define CMD_SET_MEMORY_MODE     0x20
#define CMD_SET_COLUMN_LOWER    0x00
#define CMD_SET_COLUMN_UPPER    0x10
#define CMD_SET_PAGE_START      0xB0
#define CMD_SET_SEG_REMAP_0     0xA0
#define CMD_SET_SEG_REMAP_1     0xA1 // Remap col 127 to 0
#define CMD_SET_COM_SCAN_INC    0xC0
#define CMD_SET_COM_SCAN_DEC    0xC8 // Remap rows

//
// Hardware Configuration Commands
//
#define CMD_SET_START_LINE      0x40
#define CMD_SET_DISPLAY_OFFSET  0xD3
#define CMD_SET_MULTIPLEX       0xA8
#define CMD_SET_DISPLAY_CLK_DIV 0xD5
#define CMD_SET_PRECHARGE       0xD9
#define CMD_SET_COM_PINS        0xDA
#define CMD_SET_VCOMH           0xDB
#define CMD_CHARGE_PUMP         0x8D

//
// Configuration Values (The "Magic Numbers" for arguments)
//
#define CONF_CLK_DIV_DEFAULT    0x80
#define CONF_MUX_RATIO_64       0x3F
#define CONF_OFFSET_NONE        0x00
#define CONF_START_LINE_0       0x00
#define CONF_CHARGE_PUMP_EN     0x14
#define CONF_MEM_MODE_HORIZ     0x00
#define CONF_COM_PINS_ALT       0x12
#define CONF_CONTRAST_HIGH      0xCF
#define CONF_PRECHARGE_DEFAULT  0xF1
#define CONF_VCOMH_DEFAULT      0x40

//
// Function Prototypes
//
void OLED_Init(void);
void OLED_Update(void);
void ClearScreen(void);
void SetPixel(int x, int y, uint8_t color);
void DrawVLine(int x, int y1, int y2, uint8_t pattern);
void DrawNumber(int x, int y, int num);

#endif // DISPLAY_H
