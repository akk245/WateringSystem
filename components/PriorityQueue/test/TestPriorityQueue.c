#include "unity.h"
#include "PriorityQueue.h"

TEST_CASE("add first node", "[Priority Queue]")
{
    Node* priorityQueue = NULL;
    push(&priorityQueue, (time_t) 100, (time_t) 110, 1);
    TEST_ASSERT( priorityQueue != NULL );
    
    time_t nextStartTime;
    time_t nextEndTime;
    
    peek(&priorityQueue, &nextStartTime, &nextEndTime);
    TEST_ASSERT( nextStartTime == 100 );
    TEST_ASSERT( nextEndTime == 110 );
    
}

TEST_CASE("add highest priority", "[Priority Queue]")
{ 
    Node* priorityQueue = NULL;
    /*
    time_t now;
    struct tm timeInfo;
    struct tm eightAM;
    struct tm nineAM;

    time(&now);
    localtime_r(&now,timeInfo);
    
    timeInfo->tm_minute = 0;
    timeInfo->second = 0;
    timeInfo->hour = 8;
    eightAM = timeInfo;
    timeInfo->hour = 9;
    nineAM = timeInfo;
    */
    
    push(&priorityQueue, (time_t) 100, (time_t) 110, 1);
    
    time_t nextStartTime;
    time_t nextEndTime;
    
    peek(&priorityQueue, &nextStartTime, &nextEndTime);
    TEST_ASSERT(nextStartTime == 100);
    TEST_ASSERT(nextEndTime == 110);
    
    push(&priorityQueue, (time_t) 90, (time_t) 95, 1);

    peek(&priorityQueue, &nextStartTime, &nextEndTime);
    TEST_ASSERT(nextStartTime == 90);
    TEST_ASSERT(nextEndTime == 95);
}

TEST_CASE("add medium priority", "[Priority Queue]")
{   
    Node* priorityQueue = NULL;
    push(&priorityQueue, (time_t) 100, (time_t) 110, 1);
    push(&priorityQueue, (time_t) 150, (time_t) 151, 1);
    push(&priorityQueue, (time_t) 125, (time_t) 130, 1);

    time_t nextStartTime;
    time_t nextEndTime;
    
    pop(&priorityQueue);

    peek(&priorityQueue, &nextStartTime, &nextEndTime);
    TEST_ASSERT(nextStartTime == 125);
    TEST_ASSERT(nextEndTime == 130);
}

TEST_CASE("add lowest priority", "[Priority Queue]")
{   
    Node* priorityQueue = NULL;
    push(&priorityQueue, (time_t) 100, (time_t) 110, 1);
    push(&priorityQueue, (time_t) 125, (time_t) 130, 1);
    push(&priorityQueue, (time_t) 150, (time_t) 151, 1);
    
    time_t nextStartTime;
    time_t nextEndTime;
    
    pop(&priorityQueue);

    peek(&priorityQueue, &nextStartTime, &nextEndTime);
    TEST_ASSERT(nextStartTime == 125);
    TEST_ASSERT(nextEndTime == 130);
    
    pop(&priorityQueue);

    peek(&priorityQueue, &nextStartTime, &nextEndTime);
    TEST_ASSERT(nextStartTime == 150);
    TEST_ASSERT(nextEndTime == 151);
}

TEST_CASE("repeating times", "[Priority Queue]")
{   
    Node* priorityQueue = NULL;
    push(&priorityQueue, (time_t) 100, (time_t) 110, 1);
    push(&priorityQueue, (time_t) 125, (time_t) 130, 1);
    
    time_t nextStartTime;
    time_t nextEndTime;
    
    pop(&priorityQueue);
    pop(&priorityQueue);

    peek(&priorityQueue, &nextStartTime, &nextEndTime);
    TEST_ASSERT(nextStartTime == 100 + SECONDS_IN_DAY);
    TEST_ASSERT(nextEndTime == 110 + SECONDS_IN_DAY);
}

TEST_CASE("one-shot time", "[Priority Queue]")
{   
    Node* priorityQueue = NULL;
    push(&priorityQueue, (time_t) 100, (time_t) 110, 0);
    
    time_t nextStartTime;
    time_t nextEndTime;

    TEST_ASSERT(peek(&priorityQueue, &nextStartTime, &nextEndTime));
    TEST_ASSERT(nextStartTime == 100);
    TEST_ASSERT(nextEndTime == 110);
    TEST_ASSERT(isEmpty(&priorityQueue) == false);

    pop(&priorityQueue);

    TEST_ASSERT(peek(&priorityQueue, &nextStartTime, &nextEndTime) == false);
    TEST_ASSERT(isEmpty(&priorityQueue));
    TEST_ASSERT(priorityQueue == NULL);
}

TEST_CASE("reject overlaps", "[Priority Queue]")
{   
    Node* priorityQueue = NULL;
    push(&priorityQueue, (time_t) 100, (time_t) 110, 0);
    
    time_t nextStartTime;
    time_t nextEndTime;

    TEST_ASSERT(push(&priorityQueue, (time_t) 105, (time_t) 115, 0) == false);
    TEST_ASSERT(push(&priorityQueue, (time_t) 101, (time_t) 109, 1) == false);
    TEST_ASSERT(push(&priorityQueue, (time_t) 90, (time_t) 105, 0) == false);

    pop(&priorityQueue);

    TEST_ASSERT(peek(&priorityQueue, &nextStartTime, &nextEndTime) == false);
    TEST_ASSERT(isEmpty(&priorityQueue));
    TEST_ASSERT(priorityQueue == NULL);
}

TEST_CASE("delete repeating node", "[Priority Queue]")
{   
    Node* priorityQueue = NULL;
    push(&priorityQueue, (time_t) 100, (time_t) 110, 1);
    
    time_t nextStartTime;
    time_t nextEndTime;

    TEST_ASSERT(delete(&priorityQueue, (time_t) 100));
    TEST_ASSERT(peek(&priorityQueue, &nextStartTime, &nextEndTime) == false);
    TEST_ASSERT(isEmpty(&priorityQueue));
    TEST_ASSERT(priorityQueue == NULL);
}

TEST_CASE("dump 1 node", "[Priority Queue]")
{
    Node* priorityQueue = NULL;
    push(&priorityQueue, (time_t) 100, (time_t) 110, 1);

    pqDump_t dump = dumpPQ(&priorityQueue, 1);
    TEST_ASSERT(dump.numWindows == 1);
    TEST_ASSERT(dump.windows[0].startTime == 100);
    TEST_ASSERT(dump.windows[0].endTime == 110);
}

TEST_CASE("dump 3 node", "[Priority Queue]")
{
    Node* priorityQueue = NULL;
    push(&priorityQueue, (time_t) 100, (time_t) 110, 1);
    push(&priorityQueue, (time_t) 150, (time_t) 200, 1);
    push(&priorityQueue, (time_t) 1000, (time_t) 1060, 1);

    pqDump_t dump = dumpPQ(&priorityQueue, 3);
    TEST_ASSERT(dump.numWindows == 3);
    TEST_ASSERT(dump.windows[0].startTime == 100);
    TEST_ASSERT(dump.windows[0].endTime == 110);
    TEST_ASSERT(dump.windows[1].startTime == 150);
    TEST_ASSERT(dump.windows[1].endTime == 200);
    TEST_ASSERT(dump.windows[2].startTime == 1000);
    TEST_ASSERT(dump.windows[2].endTime == 1060);
}