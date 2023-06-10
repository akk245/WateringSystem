#include "MenuManager.h"
#include "ButtonManager.h"
#include "WindowManager.h"
#include "MenuUtilities.h"
#include "utilities.h"
#include "esp_log.h"
#include "getTimeSM.h"
#include <time.h>
#include <string.h>
#include <stdint.h>

// TODO - add init states for other menus, then update exit table from main
typedef enum {INIT_MAIN, MAIN, INIT_ADD, ADD_START, ADD_END, DELETE, VIEW, SET_TIME} MenuState_t;

static MenuState_t menuState;

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
void runInitAdd(void);
void runAddStart(void);
void runAddEnd(void);
void SendAddWindowMessage(
    uint8_t hourStart,
    uint8_t minuteStart,
    uint8_t secondStart,
    uint8_t hourEnd,
    uint8_t minuteEnd,
    uint8_t secondEnd
);


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
            {"1", 0, 0},
            {"2", 0, 1},
            {"3", 0, 2},
            {"4", 0, 3}
        },
        .Exits = {
            INIT_ADD,
            DELETE,
            VIEW,
            SET_TIME
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
    ESP_LOGI("MenuManager","Running Menu State Machine");
    switch (menuState) 
    {
        case INIT_MAIN:
            runInitMain();
            break;
        case MAIN:
            runMain();
            break;
        case INIT_ADD:
            runInitAdd();
        case ADD_START:
            runAddStart();
            break;
        case ADD_END:
            runAddEnd();
            break;
        case DELETE:
            break;
        case VIEW:
            break;
        case SET_TIME:
            break;
        default:
            break;
    }
}

void runInitMain(void)
{
    ESP_LOGI("MenuMgr","Init Main Menu");
    // printout default selection with inverted colors
    Selection_t * pTmpSel = &MainPage.Selections[0];
    SendClearMessage(0);
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
    static uint8_t currentSelection = 0;

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

void runInitAdd(void)
{

    ESP_LOGI("MenuMgr","Init ADD Page");
    for (uint8_t i = 0; i < 7; i++)
    {
        SendClearMessage(i);
    }
    initGetTimeStateMachine();
    menuState = ADD_START;
}

void runAddStart(void)
{ 
    // TODO prompt for window start time to screen
    runGetTimeStateMachine();
    if (getInputTimeSmState() == DONE)
    {
        // save inputs from get time input state machine
        addWindowStartHour = hourInput;
        addWindowStartMin = minuteInput; 
        addWindowStartSec = secondInput;

        // re-init get time state machine for getting window end time
        initGetTimeStateMachine();
        menuState = ADD_END;
    }
}

void runAddEnd(void)
{
    // TODO - write prompt for window end time to screen
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

        menuState = MAIN;
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
    if ((hourEnd < currHour) ||
        ((hourEnd == currHour) && (minuteEnd < currMinute)) ||
        ((hourEnd == currHour) && (minuteEnd == currMinute) && (secondEnd <= currSecond)))
    {
        // window already passed today, add 24h to window start

        // this may look like it would cause an issue on the last day of the month
        // but mktime() should normalize the day properly
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
