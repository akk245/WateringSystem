#include "DisplayClock.h"
#include "MenuUtilities.h"
#include <time.h>
#include <stdio.h>

void runDisplayClock(void)
{
    time_t now;
    struct tm currTimeInfo;
    char * timeStr[15];
    
    // get time since epoch in seconds
    time(&now);

    // convert epoch time to calendar time
    localtime_r(&now, &currTimeInfo);
    int currHour = currTimeInfo.tm_hour;
    int currMinute = currTimeInfo.tm_min;
    int currSecond = currTimeInfo.tm_sec;

    sprintf(timeStr, "%02u:%02u:%02u", currHour, currMinute, currSecond);

    SendPrintMessage(&timeStr, 0, 7, 0);    
}
