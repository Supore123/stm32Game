#ifndef __DISPLAY_H
#define __DISPLAY_H

#include <stdint.h>

// SSD1306 Physical Dimensions
#define SSD1306_WIDTH            128
#define SSD1306_HEIGHT           64
#define SSD1306_PAGES            8
#define SSD1306_I2C_ADDR         0x78

// HUD and Game Constraints
#define HUD_Y_POSITION           48
#define FONT_WIDTH               3
#define FONT_HEIGHT              5
#define CHAR_SPACING             6

// SSD1306 Control Byte Modes
#define SSD1306_CMD_MODE         0x00
#define SSD1306_DATA_MODE        0x40

// Command Set Definitions
#define CMD_DISPLAY_OFF          0xAE
#define CMD_DISPLAY_ON           0xAF
#define CMD_SET_DISPLAY_CLK_DIV  0xD5
#define CONF_CLK_DIV_DEFAULT     0x80
#define CMD_SET_MULTIPLEX        0xA8
#define CONF_MUX_RATIO_64        0x3F
#define CMD_SET_DISPLAY_OFFSET   0xD3
#define CONF_OFFSET_NONE         0x00
#define CMD_SET_START_LINE_BASE  0x40
#define CMD_CHARGE_PUMP          0x8D
#define CONF_CHARGE_PUMP_EN      0x14
#define CMD_SET_MEMORY_MODE      0x20
#define CONF_MEM_MODE_HORIZ      0x00
#define CMD_SET_SEG_REMAP_1      0xA1
#define CMD_SET_COM_SCAN_DEC     0xC8
#define CMD_SET_COM_PINS         0xDA
#define CONF_COM_PINS_ALT        0x12
#define CMD_SET_CONTRAST         0x81
#define CONF_CONTRAST_HIGH       0xCF
#define CMD_SET_PRECHARGE        0xD9
#define CONF_PRECHARGE_DEFAULT   0xF1
#define CMD_SET_VCOMH            0xDB
#define CONF_VCOMH_DEFAULT       0x40
#define CMD_DISPLAY_ALL_ON       0xA4
#define CMD_DISPLAY_NORMAL       0xA6
#define CMD_SET_PAGE_START_BASE  0xB0
#define CMD_SET_COLUMN_LOWER     0x00
#define CMD_SET_COLUMN_UPPER     0x10

// Function Prototypes
void OLED_Init(void);
void OLED_Update(void);
void ClearScreen(void);
void SetPixel(int x, int y, uint8_t color);

// Draing Functions
void DrawVLine(int x, int y1, int y2, uint8_t pattern);
void DrawChar(int x, int y, char c);
void DrawNumber(int x, int y, int num);
void DrawString(int x, int y, const char* str);
void DrawRect(int x, int y, int w, int h);
void DrawBigTitle(int x, int y); // Custom "DOOM" logo drawer
void DrawLine(int x0, int y0, int x1, int y1);

#endif
