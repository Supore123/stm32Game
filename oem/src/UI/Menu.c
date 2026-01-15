#include "Menu.h"
#include "display.h"

void UI_DrawMenu(int selection)
{
    ClearScreen();

    // 1. Draw Title (Simple "DOOM" or "MENU")
    DrawBigTitle(42,5);

    // 2. Draw Options
    DrawString(40, 30, "START GAME");
    DrawString(40, 45, "LOAD SAVE");

    // 3. Draw Selection Frame (The "Frame around current option")
    if (selection == 0) { // Start Selected
        DrawRect(35, 28, 60, 9);
    } else { // Load Selected
        DrawRect(35, 43, 60, 9);
    }
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
