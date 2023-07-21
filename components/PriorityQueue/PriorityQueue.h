#ifndef PRIORITYQUEUE
#define PRIORITYQUEUE

#include "freertos/FreeRTOS.h"

#define SECONDS_IN_DAY 86400

typedef struct priorityNode {
    time_t startTime;
    time_t endTime;
    bool repeating;
    struct priorityNode *next;
} Node;

bool push(Node** head, time_t startTime, time_t endTime,bool repeating);
bool pop(Node** head);
bool peek(Node** head, time_t *pStartTime, time_t *pEndTime);
bool isEmpty(Node** head);
bool delete(Node ** head, time_t startTime);
Node* createNewNode(time_t startTime, time_t endTime,bool repeating);

#endif
