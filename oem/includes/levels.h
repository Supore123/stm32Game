#ifndef INCLUDES_LEVELS_H_
#define INCLUDES_LEVELS_H_

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
    float x, y;
} EnemySpawn_t; // Defines where they start in the level

typedef struct {
    uint8_t map[MAP_W][MAP_H];
    float start_x;
    float start_y;
    float start_angle;
    // New Enemy Data
    int enemy_count;
    EnemySpawn_t enemies[5]; // Max 5 enemies per level for performance
} Level_t;

// extern Level structures
extern const Level_t* AllLevels[];
extern const int TOTAL_LEVELS;

// API Prototypes
void Game_Init(void);
void Game_LoadLevel(int level_index);
void Game_Update(void); // Call this in your Logic Task

#endif /* INCLUDES_LEVELS_H_ */
