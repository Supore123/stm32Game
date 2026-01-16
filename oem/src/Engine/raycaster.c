#include "raycaster.h"
#include "display.h"
#include "levels.h"
#include "gameLogic.h"
#include <math.h>
#include <stdlib.h> // Required for abs()

extern GameState_t Game;

// 16x16 Skull Sprite (1 = White Pixel, 0 = Transparent)
// Visualized:
//    . . . . X X X X X X X . . . . .
//    . . . X X X X X X X X X . . . .
//    . . X X X . . X X . . X X . . .
//    . . X X X . . X X . . X X . . .
//    . . X X X X X X X X X X X . . .
//    . . X X X X X X X X X X X . . .
//    . . X X X X X X X X X X X . . .
//    . . . X X X X X X X X X . . . .
//    . . . X . X . X . X . X . . . .
static const uint16_t EnemySprite[16] = {
    0x07E0, // 0000 0111 1110 0000
    0x0FF0, // 0000 1111 1111 0000
    0x1C38, // 0001 1100 0011 1000 (Eyes)
    0x1C38, // 0001 1100 0011 1000
    0x1FF8, // 0001 1111 1111 1000
    0x1FF8, // 0001 1111 1111 1000
    0x1FF8, // 0001 1111 1111 1000
    0x0FF0, // 0000 1111 1111 0000
    0x0AA0, // 0000 1010 1010 0000 (Teeth)
    0x0AA0,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000
};

// Global Z-Buffer to track wall distances for sprite occlusion
static float ZBuffer[SSD1306_WIDTH];

//
// Performs the Vector/Wireframe Raycasting loop
//
void Render_3D_View(void)
{
	// Iterate through every vertical column of the screen
	for (int x = 0; x < SSD1306_WIDTH; x++)
	{
		// ============================================
		// 1. Ray Casting Math
		// ============================================
		float cameraX = 2 * x / (float)SSD1306_WIDTH - 1;
		float rayDirX = Game.player.dir_x + Game.player.plane_x * cameraX;
		float rayDirY = Game.player.dir_y + Game.player.plane_y * cameraX;

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

		// DDA Loop
		while (hit == 0)
		{
			if (sideDistX < sideDistY) { sideDistX += deltaDistX; mapX += stepX; side = 0; }
			else { sideDistY += deltaDistY; mapY += stepY; side = 1; }

			// Check for wall hit
			if (Game.current_level->map[mapX][mapY] > 0) hit = 1;
		}

		// Calculate final perpendicular distance
		if (side == 0) perpWallDist = (mapX - Game.player.x + (1 - stepX) / 2) / rayDirX;
		else           perpWallDist = (mapY - Game.player.y + (1 - stepY) / 2) / rayDirY;

		// --- FIX: Store Z-Buffer value AFTER calculation ---
		ZBuffer[x] = perpWallDist;

		// ============================================
		// 2. Vector / Wireframe Rendering Logic
		// ============================================

		// Calculate line height
		int lineHeight = (int)(SSD1306_HEIGHT / perpWallDist);
		int drawStart = -lineHeight / 2 + SSD1306_HEIGHT / 2;
		int drawEnd = lineHeight / 2 + SSD1306_HEIGHT / 2;

		// OPTIMIZATION: Clamp drawing to screen bounds
		int loopStart = (drawStart < 0) ? 0 : drawStart;
		int loopEnd   = (drawEnd >= SSD1306_HEIGHT) ? (SSD1306_HEIGHT - 1) : drawEnd;

		// A. Draw the "Ceiling" and "Floor" edges (Perspective Lines)
		SetPixel(x, drawStart, 1);
		SetPixel(x, drawEnd, 1);

		// B. Determine Wall Type
		uint8_t tileID = Game.current_level->map[mapX][mapY];

		// C. Draw Visual Style based on Wall Type/Side
		if (tileID == 9)
		{
			// EXIT TILE: Draw a distinct "Prison Bar" pattern
			for (int y = loopStart; y <= loopEnd; y += 2) {
				SetPixel(x, y, 1);
			}
		}
		else if (side == 1)
		{
			// SIDE WALLS (North/South): Draw sparse dotted line for depth
			for (int y = loopStart; y <= loopEnd; y += 4) {
				SetPixel(x, y, 1);
			}
		}
	}
}

//
// Casts a single ray (used for Shooting/Hitscan logic)
//
//
// Casts a single ray (used for Shooting/Hitscan logic)
// FIX: Added bounds checking to prevent system crash when shooting into void
//
float Raycast_CastSingle(float angle_offset, uint8_t *hit_type)
{
    // 1. Calculate ray direction based on player angle and offset
    float rayDirX = Game.player.dir_x;
    float rayDirY = Game.player.dir_y;

    // Apply simple rotation if offset is non-zero (optional, usually 0 for center aim)
    if (angle_offset != 0.0f) {
        float oldDirX = rayDirX;
        rayDirX = rayDirX * cosf(angle_offset) - rayDirY * sinf(angle_offset);
        rayDirY = oldDirX * sinf(angle_offset) + rayDirY * cosf(angle_offset);
    }

    // 2. DDA Setup
    int mapX = (int)Game.player.x;
    int mapY = (int)Game.player.y;

    float deltaDistX = fabsf(1.0f / rayDirX);
    float deltaDistY = fabsf(1.0f / rayDirY);

    float sideDistX, sideDistY;
    int stepX, stepY;
    int hit = 0;
    int side;

    if (rayDirX < 0) {
        stepX = -1;
        sideDistX = (Game.player.x - mapX) * deltaDistX;
    } else {
        stepX = 1;
        sideDistX = (mapX + 1.0f - Game.player.x) * deltaDistX;
    }

    if (rayDirY < 0) {
        stepY = -1;
        sideDistY = (Game.player.y - mapY) * deltaDistY;
    } else {
        stepY = 1;
        sideDistY = (mapY + 1.0f - Game.player.y) * deltaDistY;
    }

    // 3. DDA Execution
    float distance_traveled = 0;

    // We limit distance to 20.0f to prevent infinite loops if bounds check fails
    while (hit == 0 && distance_traveled < 20.0f)
    {
        // Jump to next map square
        if (sideDistX < sideDistY) {
            distance_traveled = sideDistX;
            sideDistX += deltaDistX;
            mapX += stepX;
            side = 0;
        } else {
            distance_traveled = sideDistY;
            sideDistY += deltaDistY;
            mapY += stepY;
            side = 1;
        }

        // --- FIX: CRASH PREVENTION (Bounds Check) ---
        // If ray goes outside the map array [0..15], STOP immediately.
        // Accessing map[-1] or map[16] causes the Hard Fault reset.
        if (mapX < 0 || mapX >= MAP_W|| mapY < 0 || mapY >= MAP_H) {
            hit = 1;
            if (hit_type) *hit_type = 0; // Return 0 (Air) so we don't count it as a wall hit
            break;
        }

        // Check if ray hit a wall or entity (>0 in the map array)
        if (Game.current_level->map[mapX][mapY] > 0) {
            hit = 1;
            if (hit_type) *hit_type = Game.current_level->map[mapX][mapY];
        }
    }

    // 4. Calculate final perpendicular distance
    float perpWallDist;
    if (side == 0) perpWallDist = (mapX - Game.player.x + (1 - stepX) / 2.0f) / rayDirX;
    else           perpWallDist = (mapY - Game.player.y + (1 - stepY) / 2.0f) / rayDirY;

    return (hit) ? perpWallDist : 20.0f;
}

//
// Draws 3D sprites (Enemies) using the Z-Buffer for occlusion
//
void Render_Enemies(void)
{
    // Loop through all active enemies
    for(int i = 0; i < 5; i++)
    {
        if (!Game.enemies[i].active) continue;

        // --- 1. Sprite Projection Math (Same as before) ---
        float spriteX = Game.enemies[i].x - Game.player.x;
        float spriteY = Game.enemies[i].y - Game.player.y;

        float invDet = 1.0f / (Game.player.plane_x * Game.player.dir_y - Game.player.dir_x * Game.player.plane_y);

        float transformX = invDet * (Game.player.dir_y * spriteX - Game.player.dir_x * spriteY);
        float transformY = invDet * (-Game.player.plane_y * spriteX + Game.player.plane_x * spriteY);

        if (transformY <= 0.1f) continue;

        int spriteScreenX = (int)((SSD1306_WIDTH / 2) * (1 + transformX / transformY));
        int spriteHeight = abs((int)(SSD1306_HEIGHT / transformY));
        int spriteWidth = abs((int)(SSD1306_HEIGHT / transformY));

        int drawStartY = -spriteHeight / 2 + SSD1306_HEIGHT / 2;
        if(drawStartY < 0) drawStartY = 0;
        int drawEndY = spriteHeight / 2 + SSD1306_HEIGHT / 2;
        if(drawEndY >= SSD1306_HEIGHT) drawEndY = SSD1306_HEIGHT - 1;

        int drawStartX = -spriteWidth / 2 + spriteScreenX;
        if(drawStartX < 0) drawStartX = 0;
        int drawEndX = spriteWidth / 2 + spriteScreenX;
        if(drawEndX >= SSD1306_WIDTH) drawEndX = SSD1306_WIDTH - 1;

        // --- 2. Texture Mapping Loop ---
        for(int stripe = drawStartX; stripe < drawEndX; stripe++)
        {
            // Calculate which column of the texture (0-15) to draw
            // Math: (CurrentX - StartX) * TexWidth / TotalWidth
            int texX = (int)(256 * (stripe - (-spriteWidth / 2 + spriteScreenX)) * 16 / spriteWidth) / 256;

            // Safety clamp
            if(texX < 0) texX = 0;
            if(texX > 15) texX = 15;

            // Z-BUFFER CHECK
            if(transformY < ZBuffer[stripe])
            {
                for(int y = drawStartY; y < drawEndY; y++)
                {
                    // Calculate Y coordinate on texture (0-15)
                    // Math: (CurrentY - StartY) * TexHeight / TotalHeight
                    int d = (y) * 256 - SSD1306_HEIGHT * 128 + spriteHeight * 128;
                    int texY = ((d * 16) / spriteHeight) / 256;

                    if(texY < 0) texY = 0;
                    if(texY > 15) texY = 15;

                    // Read the bit from the sprite array
                    // We shift 1 by (15 - texX) because bit 15 is the leftmost pixel
                    uint16_t color = (EnemySprite[texY] >> (15 - texX)) & 1;

                    // Draw only if the pixel is White (1)
                    // (Leave 0 as transparent so we see walls behind legs/arms)
                    if(color != 0) {
                        SetPixel(stripe, y, 1);
                    }
                }
            }
        }
    }
}
