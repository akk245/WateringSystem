#include "utilities.h"

uint8_t incrWrapAround(uint8_t currNumber, uint8_t rangeSize)
{
    if (currNumber == rangeSize - 1)
    {
        return 0;
    }
    return currNumber + 1;
}

uint8_t decrWrapAround(uint8_t currNumber, uint8_t rangeSize)
{
    if (currNumber == 0)
    {
        return rangeSize - 1;
    }
    return currNumber - 1;
}