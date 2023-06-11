#include "ScreenManager.h"
#include "ScreenHAL.h"
#include "font.h"
#include "esp_log.h"

#include <stdbool.h>
#include <string.h>

// Divide display into grid of 8x8 bitmaps
#define DISPLAY_GRID_ROWS 8
#define DISPLAY_GRID_COLS 16
#define FRAME_BUF_COLS_PER_GRID_COL 8

QueueHandle_t ScreenMessageQueue;

static uint8_t frameBuffer[DISPLAY_GRID_ROWS][DISPLAY_GRID_COLS * FRAME_BUF_COLS_PER_GRID_COL];

// forward declarations
void clearLine(uint8_t line);
void writeStr(char * strIn, uint8_t startingCol, uint8_t line, bool invertedColor);
void writeChar(char charIn, uint8_t col, uint8_t line, bool invertedColor);
bool isAlpha(char charIn);
bool isDigit(char charIn);
void writeDigit(char digitIn, uint8_t col, uint8_t line, bool invertedColor);
void writeLetter(char alpgaIn, uint8_t col, uint8_t line, bool invertedColor);
void write8x8bitmap(uint8_t * bitMap, uint8_t col, uint8_t line, bool invertedColor);
void processPrintStrMessage(char * data);
void processClearRowMessage(char * data);
void writeStr(char * strIn, uint8_t startingCol, uint8_t line, bool invertedColor);
void writeChar(char charIn, uint8_t col, uint8_t line, bool invertedColor);
void clearLine(uint8_t line);

void initScreenManager(void)
{
    // setup i2c, setup screen via i2c
    HAL_init();

    // create message queue
    ScreenMessageQueue = xQueueCreate(20,sizeof(struct ScreenMessage));

    // clear screen by clearing frame buffer
    memset(frameBuffer, 0, sizeof(frameBuffer));

    sendScreenFrame(frameBuffer);
}

void runScreenManager(void)
{
    // check queue for messages
    struct ScreenMessage recievedMessage;
    while(xQueueReceive(ScreenMessageQueue, &recievedMessage, 0))
    {
        //ESP_LOGI("ScreenMgr","Recvd Msg");
        // process messages
        if(recievedMessage.messageID == PRINT_STR)
        {
            processPrintStrMessage(recievedMessage.data);
        }
        else if(recievedMessage.messageID == CLEAR_ROW)
        {
            processClearRowMessage(recievedMessage.data);
        }
        else
        {
            ESP_LOGI("ScreenMgr","Unknown Message");
        }
    }

    // send out fameBuffer over i2c
    sendScreenFrame(frameBuffer);
}

void processPrintStrMessage(char * data)
{
    struct PrintStrData* pMsgData = (struct PrintStrData *) data;
    writeStr(pMsgData->str, pMsgData->startingCol, pMsgData->line, pMsgData->invertedColor);
}

void processClearRowMessage(char * data)
{
    struct ClearRowData * pMsgData = (struct ClearRowData *) data;
    clearLine(pMsgData->line);
}

void clearLine(uint8_t line)
{
    memset(&frameBuffer[line][0], 0, DISPLAY_GRID_COLS * FRAME_BUF_COLS_PER_GRID_COL);
}


void writeStr(char * strIn, uint8_t startingCol, uint8_t line, bool invertedColor)
{
    for (uint8_t col = startingCol; col < DISPLAY_GRID_COLS; col++)
    {
        if (*strIn == 0)
        {
            // if we reached end of str, return
            return;
        }

        writeChar(*strIn, col, line, invertedColor);
        strIn++;       
    }
    // no further characters will fit on screen line, return
    return;
}

void writeChar(char charIn, uint8_t col, uint8_t line, bool invertedColor)
{
    if (isAlpha(charIn))
    {
        writeLetter(charIn, col, line, invertedColor);
    }
    else if (isDigit(charIn))
    {
        writeDigit(charIn, col, line, invertedColor);
    }
    else {
        switch (charIn)
        {
            case ':':
                write8x8bitmap(&colonBitmap, col, line, invertedColor);
                break;
            case ' ':
                write8x8bitmap(&spaceBitmap, col, line, invertedColor);
            default:
                // TODO log unsupported char write error
                break;
        }
    }
    
}

bool isAlpha(char charIn)
{
    if (((charIn - 'a') >= 0 && (charIn - 'a') < 26) ||
        ((charIn - 'A') >= 0 && (charIn - 'A') < 26))
    {
        return true;
    }
    return false;
}

bool isDigit(char charIn)
{
    if ((charIn - '0') >= 0 && (charIn - '0') < 10)
    {
        return true;
    }
    return false;
}

void writeLetter(char alphaIn, uint8_t col, uint8_t line, bool invertedColor)
{
    if (alphaIn >= 'a')
    {
        write8x8bitmap(alphaBitmaps[alphaIn - 'a'], col, line, invertedColor);
    }
    else 
    {
        write8x8bitmap(alphaBitmaps[alphaIn - 'A'], col, line, invertedColor);
    }
}

void writeDigit(char digitIn, uint8_t col, uint8_t line, bool invertedColor)
{
    write8x8bitmap(digitBitmaps[digitIn - '0'], col, line, invertedColor);
}

void write8x8bitmap(uint8_t * bitMap, uint8_t col, uint8_t line, bool invertedColor)
{
    if (line < 0 || line >= DISPLAY_GRID_ROWS)
    {
        // TODO, log error
        return;
    }
    if (col < 0 || col >= DISPLAY_GRID_COLS)
    {
        // TODO log error
        return;
    }

    if (invertedColor)
    {
        for (uint8_t bitMapCol = 0; bitMapCol < FRAME_BUF_COLS_PER_GRID_COL; bitMapCol++)
        {
            // use '~' the bitwise NOT operator
            frameBuffer[line][col * FRAME_BUF_COLS_PER_GRID_COL + bitMapCol] = ~(*bitMap);
            bitMap++;
        }
    }
    else
    {
        memcpy(&frameBuffer[line][col * FRAME_BUF_COLS_PER_GRID_COL], bitMap, FRAME_BUF_COLS_PER_GRID_COL);
    }
}