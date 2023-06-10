#ifndef GET_TIME_SM
#define GET_TIME_SM

#include <stdint.h>

typedef enum {HOUR, MINUTE, SECOND, DONE} InputTimeState_t;

extern uint8_t hourInput;
extern uint8_t minuteInput;
extern uint8_t secondInput;

void initGetTimeStateMachine(void);
void runGetTimeStateMachine(void);
InputTimeState_t getInputTimeSmState(void);
void startInputTimeSM(void);

#endif // GET_TIME_SM