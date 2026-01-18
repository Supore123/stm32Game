#include "display.h"
#include "i2c.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>

//
// Global Framebuffer calculated based on screen dimensions
//
static uint8_t framebuffer[(SSD1306_WIDTH * SSD1306_HEIGHT) / 8];

// ==================== OLED Functions ====================

//
// Sends a single command byte to the OLED using the thread-safe I2C wrapper
//
static void OLED_WriteCmd(uint8_t cmd)
{
    uint8_t buf[2];
    buf[0] = SSD1306_CMD_MODE;
    buf[1] = cmd;

    I2C_Write_Locked(SSD1306_I2C_ADDR, buf, 2);
}

//
// Initializes the SSD1306 OLED using defined hardware constants
//
void OLED_Init(void)
{
	I2C_Init();
    // Wait for screen hardware stabilization (100ms)
    vTaskDelay(pdMS_TO_TICKS(100));

    OLED_WriteCmd(CMD_DISPLAY_OFF);
    OLED_WriteCmd(CMD_SET_DISPLAY_CLK_DIV);
    OLED_WriteCmd(CONF_CLK_DIV_DEFAULT);
    OLED_WriteCmd(CMD_SET_MULTIPLEX);
    OLED_WriteCmd(CONF_MUX_RATIO_64);
    OLED_WriteCmd(CMD_SET_DISPLAY_OFFSET);
    OLED_WriteCmd(CONF_OFFSET_NONE);
    OLED_WriteCmd(CMD_SET_START_LINE_BASE);
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
// Flushes the RAM framebuffer to the OLED memory pages via I2C
//
void OLED_Update(void)
{
    uint8_t data_buf[SSD1306_WIDTH + 1];
    data_buf[0] = SSD1306_DATA_MODE;

    for (uint8_t page = 0; page < SSD1306_PAGES; page++)
    {
        OLED_WriteCmd(CMD_SET_PAGE_START_BASE + page);
        OLED_WriteCmd(CMD_SET_COLUMN_LOWER);
        OLED_WriteCmd(CMD_SET_COLUMN_UPPER);

        memcpy(&data_buf[1], &framebuffer[page * SSD1306_WIDTH], SSD1306_WIDTH);
        I2C_Write_Locked(SSD1306_I2C_ADDR, data_buf, SSD1306_WIDTH + 1);
    }
}

// ==================== Drawing Functions ====================

//
// Clears the local framebuffer array
//
void ClearScreen(void)
{
    memset(framebuffer, 0, sizeof(framebuffer));
}

//
// Sets or clears a specific pixel with bounds checking against screen dimensions
//
void SetPixel(int x, int y, uint8_t color)
{
    if (x < 0 || x >= SSD1306_WIDTH || y < 0 || y >= SSD1306_HEIGHT)
    {
        return;
    }

    if (color)
    {
        framebuffer[x + (y / 8) * SSD1306_WIDTH] |= (1 << (y % 8));
    }
    else
    {
        framebuffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
    }
}

//
// Draws a vertical line using a pattern to simulate texture/depth
//
void DrawVLine(int x, int y1, int y2, uint8_t pattern)
{
    if (y1 > y2)
    {
        int tmp = y1;
        y1 = y2;
        y2 = tmp;
    }

    for (int y = y1; y <= y2; y++)
    {
        // Calculate dither based on pattern constant
        SetPixel(x, y, (y + x) % pattern < (pattern / 2));
    }
}

//
// Basic 5x7 Bitmap Font for ASCII (Space to Z)
//
static const uint8_t font5x7[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // Space (0x20)
    {0x00, 0x00, 0x5F, 0x00, 0x00}, // !
    {0x00, 0x03, 0x00, 0x03, 0x00}, // "
    {0x14, 0x3E, 0x14, 0x3E, 0x14}, // #
    {0x24, 0x2A, 0x7F, 0x2A, 0x12}, // $
    {0x23, 0x13, 0x08, 0x64, 0x62}, // %
    {0x36, 0x49, 0x55, 0x22, 0x50}, // &
    {0x00, 0x05, 0x03, 0x00, 0x00}, // '
    {0x00, 0x1C, 0x22, 0x41, 0x00}, // (
    {0x00, 0x41, 0x22, 0x1C, 0x00}, // )
    {0x14, 0x08, 0x3E, 0x08, 0x14}, // *
    {0x08, 0x08, 0x3E, 0x08, 0x08}, // +
    {0x00, 0x50, 0x30, 0x00, 0x00}, // ,
    {0x08, 0x08, 0x08, 0x08, 0x08}, // -
    {0x00, 0x60, 0x60, 0x00, 0x00}, // .
    {0x20, 0x10, 0x08, 0x04, 0x02}, // /
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // 9
    {0x00, 0x36, 0x36, 0x00, 0x00}, // :
    {0x00, 0x56, 0x36, 0x00, 0x00}, // ;
    {0x08, 0x14, 0x22, 0x41, 0x00}, // <
    {0x14, 0x14, 0x14, 0x14, 0x14}, // =
    {0x00, 0x41, 0x22, 0x14, 0x08}, // >
    {0x02, 0x01, 0x51, 0x09, 0x06}, // ?
    {0x32, 0x49, 0x79, 0x41, 0x3E}, // @
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, // A
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // B
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // C
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, // D
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // E
    {0x7F, 0x09, 0x09, 0x09, 0x01}, // F
    {0x3E, 0x41, 0x49, 0x49, 0x7A}, // G
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // H
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // I
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // J
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // K
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // L
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // M
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // N
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // O
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // P
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // Q
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // R
    {0x46, 0x49, 0x49, 0x49, 0x31}, // S
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // T
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // U
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // V
    {0x3F, 0x40, 0x38, 0x40, 0x3F}, // W
    {0x63, 0x14, 0x08, 0x14, 0x63}, // X
    {0x07, 0x08, 0x70, 0x08, 0x07}, // Y
    {0x61, 0x51, 0x49, 0x45, 0x43}  // Z
};

//
// Draws a standard ASCII Character (A-Z, 0-9, Symbols)
//
void DrawChar(int x, int y, char c)
{
    // 1. Convert to uppercase (simple support)
    if(c >= 'a' && c <= 'z') {
        c -= 32;
    }

    // 2. Check bounds (Space is 0x20, Z is 0x5A)
    // We only have the table from Space to Z to save memory
    if (c < ' ' || c > 'Z') {
        return; // Unknown char
    }

    // 3. Calculate Index (ASCII value - 32)
    int index = c - 32;
    const uint8_t *glyph = font5x7[index];

    // 4. Draw the 5 columns of the character
    for (int col = 0; col < 5; col++) {
        uint8_t line = glyph[col];
        for (int row = 0; row < 7; row++) {
            if ((line >> row) & 0x01) {
                SetPixel(x + col, y + row, 1);
            }
        }
    }
}

//
// Converts an integer and renders it for the Health/Ammo HUD
//
void DrawNumber(int x, int y, int num)
{
    char buf[8];
    int i = 0;

    if (num == 0)
    {
        buf[i++] = '0';
    }

    while (num > 0 && i < 7)
    {
        buf[i++] = '0' + (num % 10);
        num /= 10;
    }

    for (int j = i - 1; j >= 0; j--)
    {
        DrawChar(x, y, buf[j]);
        x += CHAR_SPACING;
    }
}

// Draws a string using the 3x5 font
void DrawString(int x, int y, const char* str)
{
    while (*str) {
        DrawChar(x, y, *str++);
        x += CHAR_SPACING; // Use the define from display.h (was hardcoded 4)
    }
}

// Draws a hollow rectangle (frame)
void DrawRect(int x, int y, int w, int h)
{
    for (int i = x; i < x + w; i++)
    {
        SetPixel(i, y, 1);       // Top
        SetPixel(i, y + h - 1, 1); // Bottom
    }
    for (int i = y; i < y + h; i++)
    {
        SetPixel(x, i, 1);       // Left
        SetPixel(x + w - 1, i, 1); // Right
    }
}

// hardcoded "DOOM" logo with a checkerboard pattern for "Color"
void DrawHLine(int x1, int x2, int y, int color) {
    for(int x = x1; x <= x2; x++) {
        SetPixel(x, y, color);
    }
}

// Blocky "DOOM" Title
void DrawBigTitle(int x, int y)
{
    int h = 16; // Height of letters
    int w = 12; // Width of letters
    int s = 16; // Spacing between start of each letter

    // --- D ---
    // Left Pillar (Thick)
    DrawVLine(x, y, y+h, 1);
    DrawVLine(x+1, y, y+h, 1);
    // Top & Bottom Bars
    DrawHLine(x, x+w-2, y, 1);
    DrawHLine(x, x+w-2, y+h, 1);
    // Right Curve
    DrawVLine(x+w-1, y+2, y+h-2, 1);
    DrawVLine(x+w, y+2, y+h-2, 1);

    // --- O ---
    x += s;
    // Side Pillars
    DrawVLine(x, y+2, y+h-2, 1);
    DrawVLine(x+1, y+2, y+h-2, 1);
    DrawVLine(x+w-1, y+2, y+h-2, 1);
    DrawVLine(x+w, y+2, y+h-2, 1);
    // Top & Bottom
    DrawHLine(x+2, x+w-2, y, 1);
    DrawHLine(x+2, x+w-2, y+h, 1);

    // --- O ---
    x += s;
    // Side Pillars
    DrawVLine(x, y+2, y+h-2, 1);
    DrawVLine(x+1, y+2, y+h-2, 1);
    DrawVLine(x+w-1, y+2, y+h-2, 1);
    DrawVLine(x+w, y+2, y+h-2, 1);
    // Top & Bottom
    DrawHLine(x+2, x+w-2, y, 1);
    DrawHLine(x+2, x+w-2, y+h, 1);

    // --- M ---
    x += s;
    // Side Pillars
    DrawVLine(x, y, y+h, 1);
    DrawVLine(x+1, y, y+h, 1);
    DrawVLine(x+w-1, y, y+h, 1);
    DrawVLine(x+w, y, y+h, 1);
    // Diagonals (Manually drawn for precision)
    SetPixel(x+2, y+2, 1); SetPixel(x+3, y+3, 1);
    SetPixel(x+4, y+4, 1); SetPixel(x+5, y+5, 1); // Left slope
    SetPixel(x+6, y+5, 1); SetPixel(x+7, y+4, 1); // Right slope
    SetPixel(x+8, y+3, 1); SetPixel(x+9, y+2, 1);
}

void DrawLine(int x0, int y0, int x1, int y1) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        SetPixel(x0, y0, 1);

        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}
