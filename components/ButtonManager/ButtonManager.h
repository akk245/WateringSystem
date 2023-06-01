#ifndef BUTTON_MANAGER_H
#define BUTTON_MANAGER_H

#include <stdbool.h>

extern bool up_button_flag;
extern bool down_button_flag;
extern bool enter_button_flag;

void initButtonManager(void);
void executeButtonManager(void);

#endif