#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "Task1000ms.h"
#include "Task10ms.h"
#include "WindowManager.h"
#include "ButtonManager.h"

#define BLINK_GPIO 5

void app_main(void)
{
    gpio_config_t BLINK_GPIO_config = {
            (uint64_t) 1 << BLINK_GPIO,
            GPIO_MODE_OUTPUT, 
            GPIO_PULLUP_DISABLE,
            GPIO_PULLDOWN_DISABLE,
            GPIO_INTR_DISABLE};

    gpio_config(&BLINK_GPIO_config);

    init1000msTask();
    init10msTask();
    /*
    while(1) {
        if(down_button_flag)
        {
            gpio_set_level(BLINK_GPIO,1);
            down_button_flag = false;
        }
        vTaskDelay(200 / portTICK_PERIOD_MS);
        gpio_set_level(BLINK_GPIO,0);
    }
    */
    while(1) {
        // 1hz Heartbeat
        gpio_set_level(BLINK_GPIO, 0);
        vTaskDelay(900 / portTICK_PERIOD_MS);
        gpio_set_level(BLINK_GPIO, 1);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
