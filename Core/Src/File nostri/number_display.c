/*
 * number_display.c
 *
 *  Created on: Jun 18, 2025
 *      Author: br1
 */


#include "number_display.h"
#include <stdio.h>

void ssd1306_DisplayNumber(int number) {
	char s_number[3];
	sprintf(s_number, "%d", number);
	ssd1306_Fill(Black);
	ssd1306_SetCursor(45,20);
	ssd1306_WriteString(s_number, Font_16x26, White);
	ssd1306_UpdateScreen();
}

void ssd1306_DisplayString(char * string,SSD1306_Font_t Font) {
	ssd1306_Fill(Black);
	ssd1306_SetCursor(3,20);
	ssd1306_WriteString(string, Font, White);
	ssd1306_UpdateScreen();
}
