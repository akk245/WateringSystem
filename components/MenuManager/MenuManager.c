#include "MenuManager.h"
#include "ButtonManager.h"
#include "WindowManager.h"
#include "MenuUtilities.h"
#include "utilities.h"
#include "esp_log.h"
#include "getTimeSM.h"
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <stdint.h>

// TODO - add init states for other menus, then update exit table from main
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
    SET_TIME_INIT,
    SET_TIME
} MenuState_t;

static MenuState_t menuState;
static uint8_t currentSelection;

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
            VIEW,
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
    //ESP_LOGI("MenuManager","Running Menu State Machine");
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
        case VIEW:
            menuState = MAIN;
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
    //ESP_LOGI("MenuManager","Main Menu Selection: %i", currentSelection);
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
        SendClearMessage(1);
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
