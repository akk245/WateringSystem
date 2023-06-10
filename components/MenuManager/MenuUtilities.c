#include "MenuUtilities.h"
#include "ScreenManager.h"
#include <stdio.h>
#include <string.h>

void SendPrintMessage(char * strIn, uint8_t startingCol, uint8_t line, bool invertedColor)
{
    struct ScreenMessage screenMsg;
    struct PrintStrData printStrData;

    strcpy(&printStrData.str, strIn);
    printStrData.line = line;
    printStrData.startingCol = startingCol;
    printStrData.invertedColor = invertedColor;

    screenMsg.messageID = PRINT_STR;
    memcpy(&screenMsg.data, &printStrData, sizeof(printStrData));
    xQueueSend(ScreenMessageQueue, &screenMsg, 0);
}

void SendClearMessage(uint8_t line)
{
    struct ScreenMessage screenMsg;
    struct ClearRowData clearRowData;
    clearRowData.line = line;

    screenMsg.messageID = CLEAR_ROW;
    memcpy(&screenMsg.data, &clearRowData, sizeof(clearRowData));
    xQueueSend(ScreenMessageQueue, &screenMsg, 0);
}