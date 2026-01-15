#ifndef INCLUDES_LEVELS_H_
#define INCLUDES_LEVELS_H_

#ifndef __GAME_LEVELS_H
#define __GAME_LEVELS_H

#include <stdint.h>
#include "adc.h" // Use your Input API
#include "levels.h"

// Map Dimensions
#define MAP_W 16
#define MAP_H 16

// Wall Texture Types (The "4 Colors")
#define WALL_NONE   0
#define WALL_SOLID  1 // White
#define WALL_CHECK  2 // 50% Gray
#define WALL_STRIPE 3 // Vertical Lines
#define WALL_BRICK  4 // Brick pattern

//
// Level Data Structure
//
typedef struct {
    const uint8_t map[MAP_W][MAP_H]; // The grid (0=Empty, 1-4=Walls)
    float start_x;
    float start_y;
    float start_angle;
    uint8_t enemy_count;
    // You could add Enemy_t spawn_points[] here later
} Level_t;

// API Prototypes
void Game_Init(void);
void Game_LoadLevel(int level_index);
void Game_Update(void); // Call this in your Logic Task

#endif

#endif /* INCLUDES_LEVELS_H_ */
