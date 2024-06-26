#include "PriorityQueue.h"

bool push(Node** head, time_t startTime, time_t endTime, bool repeating){
    
    if (endTime <= startTime)
    {
        return false;
    }

    // if queue is empty place first node
    if (*head == NULL){
        Node* tmp = createNewNode(startTime,endTime,repeating);
        *head = tmp;
        return true;
    }
    // if this is the highest priority
    if (startTime < (*head)->startTime){
        // fail if end time overlaps with next window start
        if (endTime > (*head)->startTime){
            return false;
        }
        Node* tmp = createNewNode(startTime,endTime,repeating);
        tmp->next = (*head);
        *head = tmp;
        return true; 
    }
    else {
        // else scan through the queue to find correct priority slot
        Node* nodeToCheck = *head;
        while (nodeToCheck->next != NULL && startTime > nodeToCheck->next->startTime){
            nodeToCheck = nodeToCheck->next;
        }
        
        // fail if startTime overlaps end of prior window
        if (startTime < nodeToCheck->endTime)
        {
            return false;
        }
        
        // fail if endTime overlaps start of next window
        if (nodeToCheck->next != NULL && endTime > nodeToCheck->next->startTime)
        {
            return false;
        }

        Node* tmp = createNewNode(startTime,endTime,repeating);
        tmp->next = nodeToCheck->next;
        nodeToCheck->next = tmp;
        return true;
    }
}

bool pop(Node** head)
{
    if (isEmpty(head))
    {
        return false;
    }
    
    if ((*head)->repeating)
    {
        push(head,(*head)->startTime + SECONDS_IN_DAY,(*head)->endTime + SECONDS_IN_DAY,true);
    }

    Node* tmp = *head;
    *head = (*head)->next;
    free(tmp);
    return true;
}

bool peek(Node** head, time_t *pStartTime, time_t *pEndTime)
{
    if (isEmpty(head)){
        return false;
    }
    *pStartTime = (*head)->startTime;
    *pEndTime = (*head)->endTime;
    return true;
}

bool isEmpty(Node** head) {
    return (*head) == NULL;
}

bool delete(Node ** head, time_t startTime) {
    // fail to delete if empty
    if (isEmpty(head)){
        return false;
    }

    // special case if deleting head
    if ((*head)->startTime == startTime)
    {
        Node* tmp = *head;
        *head = (*head)->next;
        free(tmp);
        return true;
    }
    else
    {
        // scan through the queue to find matching Node
        Node* nodeToCheck = *head;
        while (nodeToCheck->next != NULL)
        {
            if (nodeToCheck->next->startTime == startTime)
            {
                Node* tmp = nodeToCheck->next;
                nodeToCheck->next = nodeToCheck->next->next;
                free(tmp);
                return true;
            }
            nodeToCheck = nodeToCheck->next;
        }

        return false;
    }
}
Node* createNewNode(time_t startTime, time_t endTime, bool repeating)
{
    Node* tmp = (Node*)malloc(sizeof(Node));
    tmp->startTime = startTime;
    tmp->endTime = endTime;
    tmp->repeating = repeating;
    tmp->next = NULL;

    return tmp;
}

pqDump_t dumpPQ(Node** head, uint8_t maxWindowsToDump)
{
    Node* nodePtr = *head;
    pqDump_t dump;
    
    int windowsProcessed = 0;
    while (nodePtr != NULL && windowsProcessed < maxWindowsToDump)
    {
        dump.windows[windowsProcessed].startTime = nodePtr->startTime;
        dump.windows[windowsProcessed].endTime = nodePtr->endTime;
        nodePtr = nodePtr->next;
        windowsProcessed++;
    }

    dump.numWindows = windowsProcessed;

    return dump;
}

// returns the number of windows added to the PQ
uint8_t populateFromDump(Node** head, pqDump_t dataIn)
{

    if (!isEmpty(head))
    {
        // can only populate an empty queue
        return 0;
    }   

    int windowsAdded = 0;

    for (uint8_t windowIndex = 0; windowIndex < dataIn.numWindows; windowIndex++)
    {
        bool success = push(head, 
            dataIn.windows[windowIndex].startTime,
            dataIn.windows[windowIndex].endTime,
            dataIn.windows[windowIndex].repeating);

        if (success)
        {
            windowsAdded++;
        }
    }

    return windowsAdded;
}