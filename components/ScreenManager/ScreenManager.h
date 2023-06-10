#ifndef SCREEN_MAANAGER_H
#define SCREEN_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

// TODO declair extern message queue handle

enum ScreenMessageID{PRINT_STR, CLEAR_ROW};

extern QueueHandle_t ScreenMessageQueue;

struct ScreenMessage
{
    char messageID; 
    char data[20];
};

struct PrintStrData
{
    char str[16];
    uint8_t startingCol;
    uint8_t line;
    bool invertedColor;
};

struct ClearRowData
{
    uint8_t line;
};

void initScreenManager(void);
void runScreenManager(void);

#endif // SCREEN_MANAGER_H