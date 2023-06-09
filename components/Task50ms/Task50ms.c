#include "Task50ms.h"
#include <string.h>
#include "ScreenManager.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TASK_50_MS_STACK_SIZE 4096
#define TASK_50_MS_PRIORITY 4
#define INCLUDE_xTaskDelayUntil 1

TaskHandle_t Task50msHandle;

void Task50msCode(void);

void init50msTask(void)
{
    initScreenManager();

    // Add a test message to the queue
    struct PrintStrData printStrData;
    strcpy(&printStrData.str, "12345 12:20");
    printStrData.line = 1;
    printStrData.startingCol = 0;
    printStrData.invertedColor = 0;

    struct ScreenMessage screenMsg;
    screenMsg.messageID = PRINT_STR;
    memcpy(&screenMsg.data, &printStrData, sizeof(printStrData));
    xQueueSend(ScreenMessageQueue, &screenMsg, 0);

    // Create Task with 10ms period
    xTaskCreate(Task50msCode,
        "50ms_task", 
        TASK_50_MS_STACK_SIZE,
        NULL,
        TASK_50_MS_PRIORITY,
        &Task50msHandle);
}

void Task50msCode(void) {
    TickType_t lastWakeTime;
    const TickType_t taskFrequency = 50 / portTICK_PERIOD_MS;
    
    lastWakeTime = xTaskGetTickCount();
    while(1)
    {
        // TODO, figure out how to include xTaskDelayUntil
        // we will use xTaskDelayUntil's return to see if we are overruning
        vTaskDelayUntil(&lastWakeTime, taskFrequency);
        
        runScreenManager();
    }
}