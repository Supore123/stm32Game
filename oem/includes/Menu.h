
#ifndef INCLUDES_MENU_H_
#define INCLUDES_MENU_H_

#include <stdint.h>

void UI_DrawString(int x, int y, const char* str);
void UI_DrawRect(int x, int y, int w, int h);
void UI_DrawTitle(int x, int y); // The Doom Logo

#endif /* INCLUDES_MENU_H_ */
