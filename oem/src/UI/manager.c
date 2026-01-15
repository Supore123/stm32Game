#include "input.h"

//
// Processes shooting when the joystick button is clicked
//
void Game_HandleCombat(void)
{
    PlayerInput_t input = Input_ReadState();

    if (input.is_firing)
    {
        // 1. Draw Muzzle Flash (Static Sprite or Dithered Circle)
        // This is drawn over the 3D view in the center
        for(int i = 0; i < 5; i++) {
            DrawVLine(64-2+i, 32-2, 32+2, 2); // Checkerboard spark
        }

        // 2. Simple Hitscan: If an enemy is in front of you, they take damage
        // We reuse the raycaster logic for the center ray (x = 64)
        // [Logic to check if ray hits Enemy ID in map]
    }
}
