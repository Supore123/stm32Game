#include "Menu.h"
#include "display.h"

/* oem/src/UI/Menu.c */
#include "Menu.h"
#include "display.h"
#include "gameLogic.h"
#include <stdio.h>

void UI_DrawMenu(MenuOption_t selected)
{
    DrawBigTitle(15, 5); // Draws game title

    // --- Menu Options ---
    if (selected == MENU_CLASSIC) {
        DrawString(30, 30, "> CLASSIC");
        DrawString(30, 42, "  ARCADE");
    } else {
        DrawString(30, 30, "  CLASSIC");
        DrawString(30, 42, "> ARCADE");
    }

    // --- High Score Display ---
    char scoreBuf[20];
    snprintf(scoreBuf, sizeof(scoreBuf), "HI-SCORE: %lu", Game.high_score);
    DrawString(10, 56, scoreBuf);
}

//
// Draws a Color Palette Test Screen
// Splits the 128px width into 4 distinct texture zones (32px each)
//
void DrawColorPalette(void)
{
    ClearScreen();

    for (int x = 0; x < SSD1306_WIDTH; x++)
    {
        for (int y = 0; y < SSD1306_HEIGHT; y++)
        {
            int color_zone = x / 32; // Divide screen into 4 columns (0-3)
            int pixel_on = 0;

            switch (color_zone)
            {
                case 0: // "Color" 1: SOLID WHITE
                    pixel_on = 1;
                    break;

                case 1: // "Color" 2: CHECKERBOARD (50% Grey)
                    // (x + y) % 2 == 0 creates a perfect grid
                    if ((x + y) % 2 == 0) pixel_on = 1;
                    break;

                case 2: // "Color" 3: VERTICAL STRIPES (Darker)
                    // Only draw every other column
                    if (x % 2 == 0) pixel_on = 1;
                    break;

                case 3: // "Color" 4: SPARSE / BRICK (Darkest)
                    // Light up 1 pixel every 4 pixels
                    if ((x % 4 == 0) && (y % 4 == 0)) pixel_on = 1;
                    break;
            }

            SetPixel(x, y, pixel_on);
        }
    }

    // Draw Labels (Optional, if your font works)
    DrawString(2,  55, "100%");
    DrawString(34, 55, "50%");
    DrawString(66, 55, "STRIP");
    DrawString(98, 55, "DOTS");

    OLED_Update();
}
