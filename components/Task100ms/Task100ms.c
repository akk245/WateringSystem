#include "Task100ms.h"
#include <string.h>
#include "ScreenManager.h"
#include "MenuManager.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TASK_100_MS_STACK_SIZE 4096
#define TASK_100_MS_PRIORITY 4
#define INCLUDE_xTaskDelayUntil 1

TaskHandle_t Task100msHandle;

void Task100msCode(void);

void init100msTask(void)
{
    initScreenManager();
    initMenuManager();

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
    xTaskCreate(Task100msCode,
        "100ms_task", 
        TASK_100_MS_STACK_SIZE,
        NULL,
        TASK_100_MS_PRIORITY,
        &Task100msHandle);
}

void Task100msCode(void) {
    TickType_t lastWakeTime;
    const TickType_t taskFrequency = 100 / portTICK_PERIOD_MS;
    
    lastWakeTime = xTaskGetTickCount();
    while(1)
    {
        // TODO, figure out how to include xTaskDelayUntil
        // we will use xTaskDelayUntil's return to see if we are overruning
        vTaskDelayUntil(&lastWakeTime, taskFrequency);
        
        runMenuManager();
        runScreenManager();
    }
}