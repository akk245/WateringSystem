#ifndef MENU_UTILITIES_H
#define MENU_UTILITIES_H

#include <stdint.h>
#include <stdbool.h>

void SendPrintMessage(char * strIn, uint8_t startingCol, uint8_t line, bool invertedColor);
void SendClearMessage(uint8_t line);

#endif // MENU_UTILITIES_H