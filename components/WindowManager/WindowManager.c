#include "WindowManager.h"
#include "WindowMgrMsgQueue.h"

#include <string.h>
#include "PriorityQueue.h"
#include "MenuManagerMsgQueue.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define MAX_NUM_WINDOWS 10
//#define PUMP_RELAY_GPIO 23
#define PUMP_RELAY_GPIO 13


QueueHandle_t MessageQueue;

bool processAddMessage(Node **head, char *data, char *numWindows);
bool processDeleteMessage(Node **head, char* data, char *numWindows);
bool processDisplayWindowsMessage(Node **head, char *numWindows);
bool processDisplayWindowsMessage(Node **head, char *numWindows);
void sendWindowsDataMessage(pqDump_t* dump);
void sendInvalidRequestMessage(void);
void runPumpStateMachine(Node **head);

void initWindowManager (void) 
{
    ESP_LOGI("WdwMgr","Initializing Window Manager");
    // create message queue
    MessageQueue = xQueueCreate(10,sizeof(struct windowMessage));
    
    // setup GPIO for output
    esp_rom_gpio_pad_select_gpio(PUMP_RELAY_GPIO);
    gpio_set_direction(PUMP_RELAY_GPIO, GPIO_MODE_OUTPUT);
    
    time_t rawtime;
    time(&rawtime);

    ESP_LOGI("WdwMgr","Raw time at Window Manager Init: %li", (long int)rawtime);
}
void executeWindowManager (void)
{
    static Node *head = NULL;
    static char numWindows = 0;

    // check queue for messages
    struct windowMessage recievedMessage;
    while(xQueueReceive(MessageQueue, &recievedMessage, 0))
    {
        // process messages
        if(recievedMessage.messageID == ADD_MESSAGE)
        {
            processAddMessage(&head, recievedMessage.data, &numWindows);
        }
        else if(recievedMessage.messageID == DELETE_MESSAGE)
        {
            processDeleteMessage(&head, recievedMessage.data, &numWindows);
        }
        else if(recievedMessage.messageID == DISPLAY_WINDOWS_MESSAGE)
        {
            processDisplayWindowsMessage(&head, &numWindows);
        }
    }
    // run state machine
    runPumpStateMachine(&head);
}

bool processAddMessage(Node **head, char *data, char *numWindows)
{
    ESP_LOGI("WdwMgr","Processing Add Message...");
    if(*numWindows >= MAX_NUM_WINDOWS)
    {
        ESP_LOGI("WdwMgr","Add Failed: max window limit");
        return false;
    }
    
    struct addMessageData *pMessageData = (struct addMessageData *)data; 
    time_t startTime = pMessageData->startTime;
    time_t endTime = pMessageData->endTime;
    bool repeating = pMessageData->repeat;

    if(push(head, startTime, endTime, repeating))
    {
        //printf("start time: %lli\n", startTime);
        ++(*numWindows);
        ESP_LOGI("WdwMgr","Adding... Start: %lli End: %lli", startTime, endTime);
        return true;
    }
    ESP_LOGI("WdwMgr","Add Failed");
    return false;
}


bool processDeleteMessage(Node **head, char* data, char *numWindows)
{   
    ESP_LOGI("WdwMgr","Processing Delete Message...");
    struct deleteMessageData *pMessageData = (struct deleteMessageData *)data; 
    time_t startTime = pMessageData->startTime;

    if (*numWindows == 0)
    {
        ESP_LOGI("WdwMgr","Delete Failed: 0 Windows Present");
        return false;
    }

    if(delete(head, startTime))
    {
        --(*numWindows);
        ESP_LOGI("WdwMgr","Delete Succesed");
        return true;
    }
    ESP_LOGI("WdwMgr","Delete Failed: no matching window");
    return false;
}

bool processDisplayWindowsMessage(Node **head, char *numWindows)
{
    if (*numWindows > 0)
    {
        // dump Priority Queue
        pqDump_t dump = dumpPQ(head, *numWindows);

        // send message to MenuManager with dump data
        sendWindowsDataMessage(&dump);
        return true;
    }
    else 
    {
        sendInvalidRequestMessage();
        return false;
    }
}

void sendWindowsDataMessage(pqDump_t* dump)
{
    struct MenuMessage message;

    message.messageID = WINDOW_DATA_MESSAGE;
    memcpy(&(message.data), dump, sizeof(pqDump_t));

    xQueueSend(MenuMessageQueue,(void*) &message, (TickType_t) 0);
}

void sendInvalidRequestMessage(void)
{
    struct MenuMessage message;
    message.messageID = INVALID_REQUEST_MESSAGE;

    xQueueSend(MenuMessageQueue,(void*) &message, (TickType_t) 0);
}

void runPumpStateMachine(Node **head)
{
    enum State{OFF, ON};
    static enum State state = OFF;

    if (isEmpty(head))
    {
        ESP_LOGI("WdwMgr","Exiting Pump State Machine: No Windows");    
        return;
    }
    
    // get current time
    time_t currentTime;
    time(&currentTime);
    
    // get start and end times from PriorityQueue
    time_t startTime;
    time_t endTime;
    peek(head, &startTime, &endTime);
    
    switch (state)
    {
    case OFF:
        if(currentTime >= startTime){
            if (currentTime >= endTime)
            {
                // fast forwarded through the window
                // due to a system time update

                // skip the window and requeue
                pop(head);
            }
            else 
            {
                gpio_set_level(PUMP_RELAY_GPIO, 1);
                ESP_LOGI("WdwMgr","Turning Pump On");
                state = ON;
            }
            
        }
        break;
    case ON:
        if( currentTime >= endTime){
            gpio_set_level(PUMP_RELAY_GPIO, 0);
            ESP_LOGI("WdwMgr","Turning Pump Off");
            state = OFF;
            
            // next window
            pop(head);
        }
        break;
    default:
        break;
    }
}