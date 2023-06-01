#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H

#include <time.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

enum MessageID{ADD_MESSAGE,DELETE_MESSAGE};

extern QueueHandle_t MessageQueue;

struct windowMessage
{
    char messageID; 
    char data[32];
};

struct addMessageData
{
    time_t startTime;
    time_t endTime;
    bool repeat;
};

struct deleteMessageData
{
    time_t startTime;
};

void initWindowManager(void);
void executeWindowManager(void);

#endif