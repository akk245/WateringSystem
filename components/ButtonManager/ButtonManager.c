#include "buttonManager.h"
#include "driver/gpio.h"

#define UP_GPIO 18
#define DOWN_GPIO 13
#define ENTER_GPIO 12

bool up_button_flag = false;
bool down_button_flag = false;
bool enter_button_flag = false;

static bool prevUpState = false;
static bool prevDownState = false;
static bool prevEnterState = false;

void initButtonManager(void)
{
    gpio_config_t GPIO_config = {
            (uint64_t) 1 << UP_GPIO | (uint64_t) 1<< DOWN_GPIO | (uint64_t) 1 << ENTER_GPIO,
            GPIO_MODE_INPUT, 
            GPIO_PULLUP_DISABLE,
            GPIO_PULLDOWN_ENABLE,
            GPIO_INTR_DISABLE};

    gpio_config(&GPIO_config);
}

void executeButtonManager(void)
{
    bool currentUpState = gpio_get_level(UP_GPIO);
    bool currentDownState = gpio_get_level(DOWN_GPIO);
    bool currentEnterState = gpio_get_level(ENTER_GPIO);

    if (currentUpState != prevUpState)
    {
        // we have a change of state
        if(currentUpState)
        {
            // rising edge
            up_button_flag = true;
        }
        prevUpState = currentUpState;
    }

    if (currentDownState != prevDownState)
    {
        // we have a change of state
        if (currentDownState)
        {
            // rising edge
            down_button_flag = true;
        }
        prevDownState = currentDownState;
    }

    if (currentEnterState != prevEnterState)
    {
        // we have a change of state
        if (currentEnterState)
        {
            // rising edge
            enter_button_flag = true;
        }
        prevEnterState = currentEnterState;
    }
}