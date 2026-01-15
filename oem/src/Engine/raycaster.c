#include "raycaster.h"
#include "display.h"
#include "levels.h"
#include <math.h>

//
// Performs the core Raycasting loop for the player's current FOV
//
void Render_3D_View(void)
{
    // Iterate through every vertical column of the screen (128 total)
    for (int x = 0; x < SSD1306_WIDTH; x++)
    {
        // 1. Calculate ray position and direction
        float cameraX = 2 * x / (float)SSD1306_WIDTH - 1;
        float rayDirX = Game.player.dir_x + Game.player.plane_x * cameraX;
        float rayDirY = Game.player.dir_y + Game.player.plane_y * cameraX;

        // 2. Setup DDA
        int mapX = (int)Game.player.x;
        int mapY = (int)Game.player.y;
        float sideDistX, sideDistY;
        float deltaDistX = fabsf(1 / rayDirX);
        float deltaDistY = fabsf(1 / rayDirY);
        float perpWallDist;
        int stepX, stepY, hit = 0, side;

        if (rayDirX < 0) { stepX = -1; sideDistX = (Game.player.x - mapX) * deltaDistX; }
        else { stepX = 1; sideDistX = (mapX + 1.0 - Game.player.x) * deltaDistX; }
        if (rayDirY < 0) { stepY = -1; sideDistY = (Game.player.y - mapY) * deltaDistY; }
        else { stepY = 1; sideDistY = (mapY + 1.0 - Game.player.y) * deltaDistY; }

        // 3. DDA loop - jump to next square until a wall is hit
        while (hit == 0)
        {
            if (sideDistX < sideDistY) { sideDistX += deltaDistX; mapX += stepX; side = 0; }
            else { sideDistY += deltaDistY; mapY += stepY; side = 1; }
            if (Game.current_level->map[mapX][mapY] > 0) hit = 1;
        }

        // 4. Calculate distance projected on camera direction
        if (side == 0) perpWallDist = (mapX - Game.player.x + (1 - stepX) / 2) / rayDirX;
        else           perpWallDist = (mapY - Game.player.y + (1 - stepY) / 2) / rayDirY;

        // 5. Calculate height of line to draw on screen
        int lineHeight = (int)(SSD1306_HEIGHT / perpWallDist);
        int drawStart = -lineHeight / 2 + SSD1306_HEIGHT / 2;
        int drawEnd = lineHeight / 2 + SSD1306_HEIGHT / 2;

        // 6. Select "Color" pattern based on distance for lighting effect
        uint8_t pattern = 1; // Solid White (Close)
        if (perpWallDist > 4.0f) pattern = 2; // Checkerboard (Mid)
        if (perpWallDist > 8.0f) pattern = 4; // Sparse (Far)

        DrawVLine(x, drawStart, drawEnd, pattern);
    }
}
