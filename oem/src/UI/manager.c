#include "gameLogic.h"
#include "input.h"
#include "raycaster.h"
#include <math.h>

GameState_t Game;

//
// Initialize the Game State on Boot
//
void Game_Init(void)
{
    Game.state = STATE_MENU;
    Game.high_score = 150; // Mock value for UI testing

    // Pre-load the first level so the background isn't null
    Game_LoadLevel(0);
    Game.state = STATE_MENU; // Ensure we stay in menu after loading
}

//
// Loads level data and sets player spawn points
//
void Game_LoadLevel(int level_index)
{
    // AllLevels is defined in level.c
    extern const Level_t* AllLevels[];
    Game.current_level = AllLevels[level_index];

    // Set Spawn Position
    Game.player.x = Game.current_level->start_x;
    Game.player.y = Game.current_level->start_y;

    // Convert starting angle to direction vectors for the raycaster
    float angle = Game.current_level->start_angle;
    Game.player.dir_x = cosf(angle);
    Game.player.dir_y = sinf(angle);

    // Plane is perpendicular to direction (for Field of View)
    // 0.66 provides a standard 66-degree FOV
    Game.player.plane_x = -0.66f * sinf(angle);
    Game.player.plane_y = 0.66f * cosf(angle);
}

//
// Main Game Logic Loop (Called by GameLogicTask)
//
void Game_Update(void)
{
    if (Game.state != STATE_PLAYING) return;

    PlayerInput_t input = Input_ReadState();

    // 1. Handle Rotation (Joystick X)
    if (fabsf(input.x) > 0.1f) {
        float rotSpeed = input.x * 0.05f;
        float oldDirX = Game.player.dir_x;
        Game.player.dir_x = Game.player.dir_x * cosf(-rotSpeed) - Game.player.dir_y * sinf(-rotSpeed);
        Game.player.dir_y = oldDirX * sinf(-rotSpeed) + Game.player.dir_y * cosf(-rotSpeed);

        float oldPlaneX = Game.player.plane_x;
        Game.player.plane_x = Game.player.plane_x * cosf(-rotSpeed) - Game.player.plane_y * sinf(-rotSpeed);
        Game.player.plane_y = oldPlaneX * sinf(-rotSpeed) + Game.player.plane_y * cosf(-rotSpeed);
    }

    // 2. Handle Movement (Joystick Y) with Collision
    if (fabsf(input.y) > 0.1f) {
        float moveSpeed = input.y * 0.08f;
        float nextX = Game.player.x + Game.player.dir_x * moveSpeed;
        float nextY = Game.player.y + Game.player.dir_y * moveSpeed;

        // Wall collision check against the map
        if (Game.current_level->map[(int)nextX][(int)Game.player.y] == 0) Game.player.x = nextX;
        if (Game.current_level->map[(int)Game.player.x][(int)nextY] == 0) Game.player.y = nextY;
    }
}

void Game_HandleCombat(void)
{
    PlayerInput_t input = Input_ReadState();

    if (input.is_firing)
    {
        uint8_t hit_id = 0;
        // Cast a ray straight ahead (0.0f offset)
        float dist = Raycast_CastSingle(0.0f, &hit_id);

        // If we hit an ID > 1 (assuming 1 is a normal wall), it's an enemy!
        if (hit_id > 1 && dist < 10.0f) {
            Game.high_score += 100; // Reward points
        }

        // Trigger muzzle flash on screen
        for(int i = 0; i < 5; i++) {
            DrawVLine(64-2+i, 32-2, 32+2, 2);
        }
    }
}
