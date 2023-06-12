#include "Task1000ms.h"
#include <string.h>
#include "WindowManager.h"
#include "DisplayClock.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "esp_log.h"

#define TASK_1000_MS_STACK_SIZE 4096
#define TASK_1000_MS_PRIORITY 3
#define INCLUDE_xTaskDelayUntil 1

TaskHandle_t Task1000msHandle;

void Task1000msCode(void);

void init1000msTask(void)
{
    initWindowManager();
    
    xTaskCreate(Task1000msCode,
        "1000ms_task", 
        TASK_1000_MS_STACK_SIZE,
        NULL,
        TASK_1000_MS_PRIORITY,
        &Task1000msHandle);
}

void Task1000msCode(void) {
    TickType_t lastWakeTime;
    const TickType_t taskFrequency = 1000 / portTICK_PERIOD_MS;
    //BaseType_t wasDelayed;

    lastWakeTime = xTaskGetTickCount();
    while(1)
    {
        // TODO, figure out how to include xTaskDelayUntil
        // we will use xTaskDelayUntil's return to see if we are overruning
        vTaskDelayUntil(&lastWakeTime, taskFrequency);
        
        executeWindowManager();
        runDisplayClock();
    }
}