#ifndef __RAYCASTER_H
#define __RAYCASTER_H

#include <stdint.h>

//
// Raycasting Math Constants
//
#define FOV_SCALE           0.66f   // Adjusts the Field of View (0.66 is standard Doom/Wolf3D)
#define MAX_RENDER_DIST     16.0f   // How far the player can see before fog/darkness
#define WALL_HEIGHT_FACTOR  1.0f    // Scaling factor for wall height on screen

//
// Texturing / Dithering Distance Thresholds
//
#define DIST_SOLID          4.0f    // Use solid pattern if closer than 4 units
#define DIST_DITHER         8.0f    // Use checkerboard if closer than 8 units
#define DIST_SPARSE         12.0f   // Use dots if closer than 12 units

// ==================== Raycaster API ====================

//
// Performs a full frame render using the Digital Differential Analyzer (DDA)
// Calculates wall distances for all 128 horizontal columns of the OLED
//
void Render_3D_View(void);

//
// Helper function to cast a single ray at a specific angle
// Used for shooting/combat (hitscan) and collision detection
//
float Raycast_CastSingle(float angle, uint8_t *hit_type);

#endif /* __RAYCASTER_H */
