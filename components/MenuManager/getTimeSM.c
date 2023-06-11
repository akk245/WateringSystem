#include "getTimeSM.h"
#include "ButtonManager.h"
#include "MenuUtilities.h"
#include "utilities.h"
#include "esp_log.h"
#include <string.h>
#include <stdio.h>

#define BLINK_PERIOD_IN_DISPATCH 10

typedef enum {NO_UNIT, HOUR_UNIT, MINUTE_UNIT, SECOND_UNIT} timeUnit_t;
static uint8_t blinkDispatchCounter;
static InputTimeState_t inputTimeState;
uint8_t hourInput;
uint8_t minuteInput;
uint8_t secondInput;

// forward declarations
void updateHourInput(void);
void updateMinuteInput(void);
void updateSecondInput(void);
void updateTimeUnit(uint8_t * pUnitIn, uint8_t unitMax);
void blinkCurrentTimeSelection(timeUnit_t timeUnitToBlink);
void printCurrentTimeSelection(timeUnit_t timeUnitToMask);


void initGetTimeStateMachine(void)
{
    ESP_LOGI("GetTimeSM","Init Get Time SM");
 
    blinkDispatchCounter = 0;
    hourInput = 0;
    minuteInput = 0;
    secondInput = 0;
    inputTimeState = HOUR;

    printCurrentTimeSelection(NO_UNIT);
}

void runGetTimeStateMachine(void)
{
    blinkDispatchCounter =  incrWrapAround(blinkDispatchCounter, BLINK_PERIOD_IN_DISPATCH);
    switch (inputTimeState)
    {   
        case HOUR:
            blinkCurrentTimeSelection(HOUR_UNIT);
            
            if (enter_button_flag)
            {
                ESP_LOGI("GetTimeSM","Enter Button Pressed In Get Time Hour");
                enter_button_flag = 0;
                inputTimeState = MINUTE;
                break;
            }
            
            updateHourInput();
            break;
        case MINUTE:
            blinkCurrentTimeSelection(MINUTE_UNIT);

            if (enter_button_flag)
            {
                ESP_LOGI("GetTimeSM","Enter Button Pressed In Get Time Min");
                enter_button_flag = 0;
                inputTimeState = SECOND;
                break;
            }
            
            updateMinuteInput();
            break;
        case SECOND:
            blinkCurrentTimeSelection(SECOND_UNIT);

            if (enter_button_flag)
            {
                ESP_LOGI("GetTimeSM","Enter Button Pressed In Get Time Sec");
                enter_button_flag = 0;
                inputTimeState = DONE;
                break;
            }
            
            updateSecondInput();
            break;
        case DONE:
            break;
        default:
            break;
    }
}

InputTimeState_t getInputTimeSmState(void)
{
    return inputTimeState;
}

void updateHourInput(void)
{
    updateTimeUnit(&hourInput, 24);
}

void updateMinuteInput(void)
{
    updateTimeUnit(&minuteInput, 60);
}

void updateSecondInput(void)
{
    updateTimeUnit(&secondInput, 60);
}

void updateTimeUnit(uint8_t * pUnitIn, uint8_t unitMax)
{   
    if (up_button_flag)
    {
        up_button_flag = 0;
        ESP_LOGI("GetTimeSM","Up Button Pressed In Get Time");
        *pUnitIn = incrWrapAround(*pUnitIn, unitMax);
    } 
    else if (down_button_flag)
    {
        down_button_flag = 0;
        ESP_LOGI("GetTimeSM","Down Button Pressed In Get Time");
        *pUnitIn = decrWrapAround(*pUnitIn, unitMax);
    }
}

void blinkCurrentTimeSelection(timeUnit_t timeUnitToBlink)
{
    if (blinkDispatchCounter > (BLINK_PERIOD_IN_DISPATCH * 3 / 4))
    {

        ESP_LOGI("GetTimeSM", "Masking");
        printCurrentTimeSelection(timeUnitToBlink);
    }
    else
    {
        printCurrentTimeSelection(NO_UNIT);
    }
}

void printCurrentTimeSelection(timeUnit_t timeUnitToMask)
{
    // make buffer large enough to avoid sprintf directive overflow warnings
    // we guaranteee, due to data type of the inputs to sprintf that we will not
    // use more than 9 chars, but the compiler is giving a warning based off the 
    // %02u directive being able to print out a 4 byte uint.
    char timeStr[15];

    switch (timeUnitToMask)
    {
        case NO_UNIT:
            sprintf(timeStr, "%02u:%02u:%02u", hourInput, minuteInput, secondInput);
            break;
        case HOUR_UNIT:
            sprintf(timeStr, "  :%02u:%02u", minuteInput, secondInput);
            break;
        case MINUTE_UNIT:
            sprintf(timeStr, "%02u:  :%02u", hourInput, secondInput);
            break;
        case SECOND_UNIT:
            sprintf(timeStr, "%02u:%02u:  ", hourInput, minuteInput);
            break;
        default:
            break;
    }
    SendPrintMessage(timeStr, 2, 5, 0);
}