#include "MenuManager.h"
#include "ButtonManager.h"
#include "WindowMgrMsgQueue.h"
#include "MenuManagerMsgQueue.h"
#include "MenuUtilities.h"
#include "utilities.h"
#include "esp_log.h"
#include "PriorityQueue.h"
#include "getTimeSM.h"
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <stdint.h>

typedef enum {
    INIT_MAIN,
    MAIN,
    ADD_INIT,
    ADD_START,
    ADD_END,
    DELETE_INIT,
    DELETE_START,
    DELETE_END,
    VIEW,
    VIEW_RESPONSE,
    VIEW_INIT,
    SET_TIME_INIT,
    SET_TIME
} MenuState_t;

static MenuState_t menuState;
static uint8_t currentSelection;
static pqDump_t windowData;

QueueHandle_t MenuMessageQueue;

static uint8_t addWindowStartHour;
static uint8_t addWindowStartMin;
static uint8_t addWindowStartSec;

typedef struct  {
    char str[16];
    uint8_t startingCol;
    uint8_t line;
} Selection_t;

typedef struct {
    uint8_t numSelections;
    Selection_t Selections[7];
    MenuState_t Exits[7];
} MenuPage_t;

MenuPage_t MainPage;
MenuPage_t AddPage;
MenuPage_t DeletePage;
MenuPage_t ViewPage;
MenuPage_t SetTimePage;

// forward declarations
void initMenuStateMachine(void);
void runMenuStateMachine(void);
void runInitMain(void);
void runMain(void);
void runAddInit(void);
void runAddStart(void);
void runAddEnd(void);
void runDeleteInit(void);
void runDeleteStart(void);
void runDeleteEnd(void);
void runViewInit(void);
void runViewResponse(void);
void runView(void);
void runSetTimeInit(void);
void runSetTime(void);
void SendAddWindowMessage(
    uint8_t hourStart,
    uint8_t minuteStart,
    uint8_t secondStart,
    uint8_t hourEnd,
    uint8_t minuteEnd,
    uint8_t secondEnd
);
void sendDeleteWindowMessage(
    uint8_t hourStart,
    uint8_t minuteStart,
    uint8_t secondStart,
    uint8_t hourEnd,
    uint8_t minuteEnd,
    uint8_t secondEnd
);
bool timeHasPassed(uint8_t hour, uint8_t min, uint8_t sec, struct tm currentTime);


// function definitions
void initMenuManager(void)
{
    MenuMessageQueue = xQueueCreate(1, sizeof(struct MenuMessage));

    initMenuStateMachine();
}

void initMenuStateMachine()
{
    // Init All page data into sturcts
    MainPage = (MenuPage_t) {
        .numSelections = 4,
        .Selections = {
            {"Add A Window", 0, 0},
            {"Delete A Window", 0, 1},
            {"View Windows", 0, 2},
            {"Set Clock", 0, 3}
        },
        .Exits = {
            ADD_INIT,
            DELETE_INIT,
            VIEW_INIT,
            SET_TIME_INIT
        }
    };
    
    
    // TODO - define page structure for other pages
    /*
    AddPage;
    MenuPage_t DeletePage;
    MenuPage_t ViewPage;
    MenuPage_t SetTimePage;
    */
    
    menuState = INIT_MAIN;
}

void runMenuManager(void)
{
    runMenuStateMachine();
}

void runMenuStateMachine(void)
{
    switch (menuState) 
    {
        case INIT_MAIN:
            runInitMain();
            break;
        case MAIN:
            runMain();
            break;
        case ADD_INIT:
            runAddInit();
        case ADD_START:
            runAddStart();
            break;
        case ADD_END:
            runAddEnd();
            break;
        case DELETE_INIT:
            runDeleteInit();
            break;
        case DELETE_START:
            runDeleteStart();
            break;
        case DELETE_END:
            runDeleteEnd();
            break;
        case VIEW_INIT:
            runViewInit();
            break;
        case VIEW_RESPONSE:
            runViewResponse();
            break;    
        case VIEW:
            runView();
            break;
        case SET_TIME_INIT:
            runSetTimeInit();
            break;
        case SET_TIME:
            runSetTime();
            break;
        default:
            break;
    }
}

void runInitMain(void)
{
    ESP_LOGI("MenuMgr","Init Main Menu");
    
    currentSelection = 0;
    
    // clear screen body
    for (uint8_t line = 0; line < 7; line++)
    {
        SendClearMessage(line);
    }

    // printout default selection with inverted colors
    Selection_t * pTmpSel = &MainPage.Selections[0];    
    SendPrintMessage(pTmpSel->str, pTmpSel->startingCol, pTmpSel->line, 1);
    
    // printout other selections
    for (uint8_t sel = 1; sel < MainPage.numSelections; sel++)
    {
        SendClearMessage(sel);
        Selection_t * pTmpSel = &MainPage.Selections[sel];
        SendPrintMessage(pTmpSel->str, pTmpSel->startingCol, pTmpSel->line, 0);
    }

    // exit to MAIN state
    menuState = MAIN;
}

void runMain(void)
{
    if (enter_button_flag)
    {
        enter_button_flag = 0;
        menuState =  MainPage.Exits[currentSelection];
        return;
    }
    
    if (up_button_flag)
    {
        ESP_LOGI("MenuMgr","Up Button On Main Menu");
        up_button_flag = 0;

        // re-print old selection without inverse color    
        Selection_t * pTmpSel = &MainPage.Selections[currentSelection];
        SendPrintMessage(pTmpSel->str, pTmpSel->startingCol, pTmpSel->line, 0);
        
        currentSelection = decrWrapAround(currentSelection, MainPage.numSelections);

        // print new selection with inverse color
        pTmpSel = &MainPage.Selections[currentSelection];
        SendPrintMessage(pTmpSel->str, pTmpSel->startingCol, pTmpSel->line, 1);

    } 
    else if (down_button_flag)
    {
        down_button_flag = 0;
        ESP_LOGI("MenuMgr","Down Button On Main Menu");

        // re-print old selection without inverse color    
        Selection_t * pTmpSel = &MainPage.Selections[currentSelection];
        SendPrintMessage(pTmpSel->str, pTmpSel->startingCol, pTmpSel->line, 0);
        
        currentSelection = incrWrapAround(currentSelection, MainPage.numSelections);

        // print new selection with inverse color
        pTmpSel = &MainPage.Selections[currentSelection];
        SendPrintMessage(pTmpSel->str, pTmpSel->startingCol, pTmpSel->line, 1);
    }
}

void runAddInit(void)
{

    ESP_LOGI("MenuMgr","Init ADD Page");
    for (uint8_t i = 0; i < 7; i++)
    {
        SendClearMessage(i);
    }
    initGetTimeStateMachine();
    SendPrintMessage("Add New Window", 0, 0 ,0);

    SendPrintMessage("Set Window Start", 0, 2 ,0);
    menuState = ADD_START;
}

void runAddStart(void)
{ 
    runGetTimeStateMachine();
    if (getInputTimeSmState() == DONE)
    {
        // save inputs from get time input state machine
        addWindowStartHour = hourInput;
        addWindowStartMin = minuteInput; 
        addWindowStartSec = secondInput;

        // prompt for window start time to screen
        SendClearMessage(2);
        SendPrintMessage("Set Window End", 0, 2 ,0);

        // re-init get time state machine for getting window end time
        initGetTimeStateMachine();
        menuState = ADD_END;
    }
}

void runAddEnd(void)
{
    runGetTimeStateMachine();
    
    if (getInputTimeSmState() == DONE)
    {
        // add window!
        SendAddWindowMessage(
            addWindowStartHour,
            addWindowStartMin,
            addWindowStartSec,
            hourInput,
            minuteInput,
            secondInput);

        menuState = INIT_MAIN;
    }
}

void runDeleteInit(void)
{
    ESP_LOGI("MenuMgr","Init DELETE Page");
    for (uint8_t i = 0; i < 7; i++)
    {
        SendClearMessage(i);
    }
    initGetTimeStateMachine();
    SendPrintMessage("Delete Window", 0, 0 ,0);

    SendPrintMessage("Enter Window", 0, 2 ,0);
    SendPrintMessage("Start Time", 0, 3 ,0);
    menuState = DELETE_START;
}

void runDeleteStart(void)
{ 
    runGetTimeStateMachine();
    if (getInputTimeSmState() == DONE)
    {
        // save inputs from get time input state machine
        addWindowStartHour = hourInput;
        addWindowStartMin = minuteInput; 
        addWindowStartSec = secondInput;

        // prompt for window start time to screen
        for (uint8_t i = 0; i < 7; i++)
        {
            SendClearMessage(i);
        }
        SendPrintMessage("Enter Window End", 0, 1 ,0);

        // re-init get time state machine for getting window end time
        initGetTimeStateMachine();
        menuState = DELETE_END;
    }
}

void runDeleteEnd(void)
{
    runGetTimeStateMachine();

    if (getInputTimeSmState() == DONE)
    {
        // delete window!
        sendDeleteWindowMessage(
            addWindowStartHour,
            addWindowStartMin,
            addWindowStartSec,
            hourInput,
            minuteInput,
            secondInput);

        menuState = INIT_MAIN;
    }
}

void runViewInit(void)
{
    // clear screen
    for (uint8_t i = 0; i < 7; i++)
    {
        SendClearMessage(i);
    }

    SendPrintMessage("Fetching Windows", 0, 1 ,0);

    // send a dump message to window manager
    struct windowMessage message;
    message.messageID = DISPLAY_WINDOWS_MESSAGE;

    xQueueSend(MessageQueue,(void*) &message, (TickType_t) 0);

    menuState = VIEW_RESPONSE;
}

void runViewResponse(void)
{
    currentSelection = 0;

    struct MenuMessage recievedMessage;

    if (enter_button_flag)
    {
        enter_button_flag = 0;
        menuState =  INIT_MAIN;
        return;
    }

    if(xQueueReceive(MenuMessageQueue, &recievedMessage, 0))
    {
        menuState = VIEW;
        // process messages
        if(recievedMessage.messageID == WINDOW_DATA_MESSAGE)
        {
            ESP_LOGI("MenuMgr","Recieved WINDOW_DATA message");
            memcpy(&windowData, &(recievedMessage.data), sizeof(pqDump_t));
        }
        else if(recievedMessage.messageID == INVALID_REQUEST_MESSAGE)
        {
            ESP_LOGI("MenuMgr","Recieved INVALID_REQUEST message");
            windowData.numWindows = 0;
        }
    }
}

void runView(void)
{
    if (enter_button_flag)
    {
        enter_button_flag = 0;
        menuState =  INIT_MAIN;
        return;
    }

    if (windowData.numWindows > 0)
    {
        if (up_button_flag)
        {
            ESP_LOGI("MenuMgr","Up Button On View Windows Menu");
            up_button_flag = 0;

            currentSelection = decrWrapAround(currentSelection, windowData.numWindows);
        } 
        else if (down_button_flag)
        {
            down_button_flag = 0;
            ESP_LOGI("MenuMgr","Down Button On View Windows Menu");

            currentSelection = incrWrapAround(currentSelection, windowData.numWindows);
        }

        // clear screen
        for (uint8_t i = 0; i < 7; i++)
        {
            SendClearMessage(i);
        }


        struct tm currTimeInfo;
    
        char * startTimeStr[15];
        char * endTimeStr[15];

        // convert start time from epoch to calendar time
        localtime_r(&windowData.windows[currentSelection].startTime, &currTimeInfo);
        int currHour = currTimeInfo.tm_hour;
        int currMinute = currTimeInfo.tm_min;
        int currSecond = currTimeInfo.tm_sec;

        sprintf(startTimeStr, "%02u:%02u:%02u", currHour, currMinute, currSecond);
        
        
        // convert end time from epoch to calendar time
        localtime_r(&windowData.windows[currentSelection].endTime, &currTimeInfo);
        currHour = currTimeInfo.tm_hour;
        currMinute = currTimeInfo.tm_min;
        currSecond = currTimeInfo.tm_sec;

        sprintf(endTimeStr, "%02u:%02u:%02u", currHour, currMinute, currSecond);

        char titleStr[16];
        sprintf(titleStr,"Window %i",currentSelection + 1);

        SendPrintMessage(titleStr, 0, 0, 1);
        SendPrintMessage("Start Time:", 0, 1, 0);
        SendPrintMessage(startTimeStr, 0, 2, 0);
        SendPrintMessage("End Time:", 0, 3, 0);
        SendPrintMessage(endTimeStr, 0, 4, 0);
        SendPrintMessage("Press Enter", 0, 5, 1);
        SendPrintMessage("To Return", 0, 6, 1);
    }
    else
    {
        SendPrintMessage("Zero Windows", 0, 3, 0);
    }
}

void runSetTimeInit(void)
{    
    ESP_LOGI("MenuMgr","Init SetTime Page");
    for (uint8_t i = 0; i < 7; i++)
    {
        SendClearMessage(i);
    }
    initGetTimeStateMachine();
    SendPrintMessage("Set Time", 0, 0 ,0);

    SendPrintMessage("Enter New", 0, 2 ,0);
    SendPrintMessage("System Time", 0, 3 ,0);
    menuState = SET_TIME;
}

void runSetTime(void)
{
    runGetTimeStateMachine();

    if (getInputTimeSmState() == DONE)
    {
        time_t now;
        struct tm newTimeInfo;
        struct tm currTimeInfo;
        
        // get time since epoch in seconds
        time(&now);

        // convert epoch time to calendar time
        localtime_r(&now, &currTimeInfo);
        int currHour = currTimeInfo.tm_hour;
        int currMinute = currTimeInfo.tm_min;
        int currSecond = currTimeInfo.tm_sec;
    
        if (timeHasPassed(hourInput, minuteInput, secondInput, currTimeInfo))
        {
            // if time has passed today, set time equal to the time next day
            // essentially, we want to go forward to the requested time next day
            // and not back in time to earlier today
            currTimeInfo.tm_mday++;
        }

        currTimeInfo.tm_hour = hourInput;
        currTimeInfo.tm_min = minuteInput;
        currTimeInfo.tm_sec = secondInput;
            
        time_t newTime = mktime(&currTimeInfo);
        
        if (newTime == -1)
        {
            ESP_LOGI("MenuManager","Failed to Set New Time");
            menuState = INIT_MAIN;
            return;
        }
        
        struct timeval newTimeTV;

        newTimeTV.tv_sec = newTime;
        newTimeTV.tv_usec = 0;

        settimeofday(&newTimeTV, NULL);

        menuState = INIT_MAIN;
    }
}

void SendAddWindowMessage(
    uint8_t hourStart,
    uint8_t minuteStart,
    uint8_t secondStart,
    uint8_t hourEnd,
    uint8_t minuteEnd,
    uint8_t secondEnd)
{
    time_t now;
    struct tm currTimeInfo;
    struct tm startTimeInfo;
    struct tm endTimeInfo;

    // get time since epoch in seconds
    time(&now);

    // convert epoch time to calendar time
    localtime_r(&now, &currTimeInfo);
    int currHour = currTimeInfo.tm_hour;
    int currMinute = currTimeInfo.tm_min;
    int currSecond = currTimeInfo.tm_sec;

    // if end of window is in the past
    if (timeHasPassed(hourEnd, minuteEnd, secondEnd, currTimeInfo))
    {
        // window already passed today, add 24h to window start

        // this may look like it would cause an issue on the last day of the month
        // but mktime() should normalizes its inputs
        currTimeInfo.tm_mday++;   
    }
   
    memcpy(&startTimeInfo, &currTimeInfo, sizeof(currTimeInfo));
    memcpy(&endTimeInfo, &currTimeInfo, sizeof(currTimeInfo));

    startTimeInfo.tm_hour = hourStart;
    startTimeInfo.tm_min = minuteStart;
    startTimeInfo.tm_sec = secondStart;

    endTimeInfo.tm_hour = hourEnd;
    endTimeInfo.tm_min = minuteEnd;
    endTimeInfo.tm_sec = secondEnd;

    time_t startTime = mktime(&startTimeInfo);
    time_t endTime = mktime(&endTimeInfo);


    ESP_LOGI("MenuManager", "Hour Start: %i, Min Start: %i, Second Start: %i", hourStart, minuteStart, secondStart);
    ESP_LOGI("MenuManager", "Hour End: %i, Min End: %i, Second End: %i", hourEnd, minuteEnd, secondEnd);

    if (startTime == -1 || endTime == -1)
    {
        ESP_LOGI("MenuManager", "Failed to make epoch time based on user input!");
        return;
    }

    struct windowMessage message;
    struct addMessageData messageData;
    messageData.startTime = startTime;
    messageData.endTime = endTime;
    messageData.repeat = true;

    message.messageID = ADD_MESSAGE;
    memcpy(&(message.data), &messageData, sizeof(messageData));

    xQueueSend(MessageQueue,(void*) &message, (TickType_t) 0);
}

void sendDeleteWindowMessage(
    uint8_t hourStart,
    uint8_t minuteStart,
    uint8_t secondStart,
    uint8_t hourEnd,
    uint8_t minuteEnd,
    uint8_t secondEnd)
{
    time_t now;
    struct tm currTimeInfo;
    struct tm startTimeInfo;

    // get time since epoch in seconds
    time(&now);

    // convert epoch time to calendar time
    localtime_r(&now, &currTimeInfo);
    int currHour = currTimeInfo.tm_hour;
    int currMinute = currTimeInfo.tm_min;
    int currSecond = currTimeInfo.tm_sec;

    // if end of window is in the past
    if (timeHasPassed(hourEnd, minuteEnd, secondEnd, currTimeInfo))
    {
        // window already passed today, add 24h to window start

        // this may look like it would cause an issue on the last day of the month
        // but mktime() normalizes its inputs
        currTimeInfo.tm_mday++;   
    }
   
    memcpy(&startTimeInfo, &currTimeInfo, sizeof(currTimeInfo));

    startTimeInfo.tm_hour = hourStart;
    startTimeInfo.tm_min = minuteStart;
    startTimeInfo.tm_sec = secondStart;

    time_t startTime = mktime(&startTimeInfo);

    if (startTime == -1)
    {
        ESP_LOGI("MenuManager", "Failed to create start time for delete");
    }
    struct windowMessage message;
    struct deleteMessageData messageData;
    messageData.startTime = startTime;

    message.messageID = DELETE_MESSAGE;
    memcpy(&(message.data), &messageData, sizeof(messageData));

    xQueueSend(MessageQueue,(void*) &message, (TickType_t) 0);
}

bool timeHasPassed(uint8_t hour, uint8_t min, uint8_t sec, struct tm currentTime)
{
    uint8_t currHour = currentTime.tm_hour;
    uint8_t currMinute = currentTime.tm_min;
    uint8_t currSecond = currentTime.tm_sec;

    return (
        (hour < currHour) ||
        ((hour == currHour) && (min < currMinute)) ||
        ((hour == currHour) && (min == currMinute) && (sec <= currSecond))
    );
}
