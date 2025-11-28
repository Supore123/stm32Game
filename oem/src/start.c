#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "queue.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>

// Display parameters
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define RENDER_HEIGHT 48  // Leave room for HUD

// I2C OLED SSD1306
#define SSD1306_ADDR 0x78
#define SSD1306_CMD 0x00
#define SSD1306_DATA 0x40

// Game constants
#define MAP_WIDTH 8
#define MAP_HEIGHT 8
#define FOV 60.0f
#define MOVE_SPEED 0.05f
#define ROT_SPEED 0.05f
#define MAX_DEPTH 8.0f

// Player state
typedef struct {
    float x, y;
    float angle;
    int health;
    int ammo;
    uint8_t shooting;
} Player;

// Enemy state
typedef struct {
    float x, y;
    uint8_t alive;
    uint8_t type;
} Enemy;

// Global variables
static uint8_t framebuffer[SCREEN_WIDTH * SCREEN_HEIGHT / 8];
static Player player = {1.5f, 1.5f, 0.0f, 100, 50, 0};
static Enemy enemies[4];

// Game map (1 = wall, 0 = empty, 2 = different texture)
static const uint8_t worldMap[MAP_WIDTH][MAP_HEIGHT] = {
    {1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,1},
    {1,0,2,0,0,2,0,1},
    {1,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,1},
    {1,0,2,0,0,2,0,1},
    {1,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1}
};

// Joystick readings
static volatile uint16_t joyX = 2048, joyY = 2048;
static volatile uint8_t joyButton = 0;

// Task handles
extern TaskHandle_t renderTaskHandle;
extern TaskHandle_t gameLogicTaskHandle;
extern TaskHandle_t inputTaskHandle;


// ==================== Raycasting Engine ====================

float fastSin(float x) {
    // Taylor series approximation
    float x2 = x * x;
    return x * (1.0f - x2 * (1.0f / 6.0f - x2 / 120.0f));
}

float fastCos(float x) {
    return fastSin(x + 1.5708f);
}

void RenderFrame(void) {
    ClearScreen();

    float rayAngle = player.angle - (FOV * 0.00872665f); // FOV/2 in radians
    float rayStep = (FOV * 0.0174533f) / SCREEN_WIDTH;

    for (int x = 0; x < SCREEN_WIDTH; x++) {
        float rayDirX = fastCos(rayAngle);
        float rayDirY = fastSin(rayAngle);

        int mapX = (int)player.x;
        int mapY = (int)player.y;

        float deltaDistX = (rayDirX == 0) ? 1e30 : fabsf(1.0f / rayDirX);
        float deltaDistY = (rayDirY == 0) ? 1e30 : fabsf(1.0f / rayDirY);

        float sideDistX, sideDistY;
        int stepX, stepY;

        if (rayDirX < 0) {
            stepX = -1;
            sideDistX = (player.x - mapX) * deltaDistX;
        } else {
            stepX = 1;
            sideDistX = (mapX + 1.0f - player.x) * deltaDistX;
        }

        if (rayDirY < 0) {
            stepY = -1;
            sideDistY = (player.y - mapY) * deltaDistY;
        } else {
            stepY = 1;
            sideDistY = (mapY + 1.0f - player.y) * deltaDistY;
        }

        int hit = 0, side, wallType = 0;
        float perpWallDist;

        while (!hit && mapX >= 0 && mapX < MAP_WIDTH && mapY >= 0 && mapY < MAP_HEIGHT) {
            if (sideDistX < sideDistY) {
                sideDistX += deltaDistX;
                mapX += stepX;
                side = 0;
            } else {
                sideDistY += deltaDistY;
                mapY += stepY;
                side = 1;
            }

            if (mapX >= 0 && mapX < MAP_WIDTH && mapY >= 0 && mapY < MAP_HEIGHT) {
                if (worldMap[mapX][mapY] > 0) {
                    hit = 1;
                    wallType = worldMap[mapX][mapY];
                }
            }
        }

        if (hit) {
            if (side == 0) perpWallDist = (mapX - player.x + (1 - stepX) / 2) / rayDirX;
            else perpWallDist = (mapY - player.y + (1 - stepY) / 2) / rayDirY;

            if (perpWallDist < 0.1f) perpWallDist = 0.1f;

            int lineHeight = (int)(RENDER_HEIGHT / perpWallDist);
            int drawStart = (RENDER_HEIGHT - lineHeight) / 2;
            int drawEnd = drawStart + lineHeight;

            if (drawStart < 0) drawStart = 0;
            if (drawEnd >= RENDER_HEIGHT) drawEnd = RENDER_HEIGHT - 1;

            uint8_t pattern = (wallType == 2) ? 3 : 2;
            if (side == 1) pattern *= 2;

            DrawVLine(x, drawStart, drawEnd, pattern);
        }

        rayAngle += rayStep;
    }

    // Draw enemies
    for (int i = 0; i < 4; i++) {
        if (!enemies[i].alive) continue;

        float dx = enemies[i].x - player.x;
        float dy = enemies[i].y - player.y;
        float dist = sqrtf(dx * dx + dy * dy);

        float angle = atan2f(dy, dx) - player.angle;
        while (angle > 3.14159f) angle -= 6.28318f;
        while (angle < -3.14159f) angle += 6.28318f;

        if (fabsf(angle) < (FOV * 0.00872665f) && dist < MAX_DEPTH) {
            int screenX = (int)(SCREEN_WIDTH / 2 * (1 + angle / (FOV * 0.00872665f)));
            int spriteHeight = (int)(RENDER_HEIGHT / dist);
            int drawStart = (RENDER_HEIGHT - spriteHeight) / 2;
            int drawEnd = drawStart + spriteHeight;

            for (int y = drawStart; y < drawEnd && y < RENDER_HEIGHT; y++) {
                if (screenX >= 0 && screenX < SCREEN_WIDTH) {
                    SetPixel(screenX, y, 1);
                    if (screenX + 1 < SCREEN_WIDTH) SetPixel(screenX + 1, y, 1);
                }
            }
        }
    }

    // HUD
    for (int x = 0; x < SCREEN_WIDTH; x++) {
        SetPixel(x, RENDER_HEIGHT, 1);
    }

    DrawNumber(2, RENDER_HEIGHT + 2, player.health);
    DrawNumber(70, RENDER_HEIGHT + 2, player.ammo);

    // Weapon sprite (bottom)
    for (int x = 50; x < 78; x++) {
        for (int y = RENDER_HEIGHT + 10; y < SCREEN_HEIGHT - 2; y++) {
            if ((x - 50) % 4 < 2 && (y - RENDER_HEIGHT - 10) % 3 < 2) {
                SetPixel(x, y, 1);
            }
        }
    }
}



// ==================== Game Logic ====================

void InitGame(void) {
    // Initialize enemies
    enemies[0] = (Enemy){3.5f, 3.5f, 1, 0};
    enemies[1] = (Enemy){5.5f, 2.5f, 1, 0};
    enemies[2] = (Enemy){2.5f, 5.5f, 1, 0};
    enemies[3] = (Enemy){6.5f, 5.5f, 1, 0};
}

void UpdateGame(void) {
    // Read joystick
    joyX = ADC_Read(0);
    joyY = ADC_Read(1);
    joyButton = (GPIOB->IDR & GPIO_IDR_IDR_3) ? 0 : 1; // D3 = PB3

    // Movement
    float moveX = 0, moveY = 0;

    if (joyY < 1500) { // Forward
        moveX = fastCos(player.angle) * MOVE_SPEED;
        moveY = fastSin(player.angle) * MOVE_SPEED;
    } else if (joyY > 2500) { // Backward
        moveX = -fastCos(player.angle) * MOVE_SPEED;
        moveY = -fastSin(player.angle) * MOVE_SPEED;
    }

    // Rotation
    if (joyX < 1500) player.angle -= ROT_SPEED;
    if (joyX > 2500) player.angle += ROT_SPEED;

    // Collision detection
    int newX = (int)(player.x + moveX);
    int newY = (int)(player.y + moveY);

    if (worldMap[newX][(int)player.y] == 0) player.x += moveX;
    if (worldMap[(int)player.x][newY] == 0) player.y += moveY;

    // Shooting
    if (joyButton && player.ammo > 0 && !player.shooting) {
        player.shooting = 1;
        player.ammo--;

        // Check if enemy hit
        for (int i = 0; i < 4; i++) {
            if (!enemies[i].alive) continue;
            float dx = enemies[i].x - player.x;
            float dy = enemies[i].y - player.y;
            float angle = atan2f(dy, dx) - player.angle;
            while (angle > 3.14159f) angle -= 6.28318f;
            while (angle < -3.14159f) angle += 6.28318f;

            if (fabsf(angle) < 0.2f) {
                enemies[i].alive = 0;
            }
        }
    } else if (!joyButton) {
        player.shooting = 0;
    }
}

// ==================== FreeRTOS Tasks ====================

void InputTask(void *params) {
    while (1) {
        UpdateGame();
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void GameLogicTask(void *params) {
    while (1) {
        // Enemy AI could go here
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void RenderTask(void *params) {
    while (1) {
        RenderFrame();
        OLED_Update();
        vTaskDelay(pdMS_TO_TICKS(50)); // ~20 FPS
    }
}

///* add threads, ... */
//I2C_Init();
//ADC_Init();
//OLED_Init();
//InitGame();
//
//// Create FreeRTOS tasks
//xTaskCreate(InputTask, "Input", 256, NULL, 2, &inputTaskHandle);
//xTaskCreate(GameLogicTask, "Logic", 256, NULL, 1, &gameLogicTaskHandle);
//xTaskCreate(RenderTask, "Render", 512, NULL, 3, &renderTaskHandle);
//
//// Start scheduler
//vTaskStartScheduler();
