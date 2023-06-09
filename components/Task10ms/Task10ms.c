#include "Task10ms.h"
#include <string.h>
#include "ButtonManager.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TASK_10_MS_STACK_SIZE 4096
#define TASK_10_MS_PRIORITY 5
#define INCLUDE_xTaskDelayUntil 1

TaskHandle_t Task10msHandle;

void Task10msCode(void);

void init10msTask(void)
{
    initButtonManager();
    // Create Task with 10ms period
    xTaskCreate(Task10msCode,
        "10ms_task", 
        TASK_10_MS_STACK_SIZE,
        NULL,
        TASK_10_MS_PRIORITY,
        &Task10msHandle);
}

void Task10msCode(void) {
    TickType_t lastWakeTime;
    const TickType_t taskFrequency = 10 / portTICK_PERIOD_MS;
    
    lastWakeTime = xTaskGetTickCount();
    while(1)
    {
        // TODO, figure out how to include xTaskDelayUntil
        // we will use xTaskDelayUntil's return to see if we are overruning
        vTaskDelayUntil(&lastWakeTime, taskFrequency);
        
        executeButtonManager();
    }
}