#ifndef UTILITIES_H
#define UTILITIES_H
#include <stdint.h>

uint8_t incrWrapAround(uint8_t currNumber, uint8_t rangeSize);
uint8_t decrWrapAround(uint8_t currNumber, uint8_t rangeSize);

#endif // UTILITIES_H