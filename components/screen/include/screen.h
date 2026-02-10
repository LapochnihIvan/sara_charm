#ifndef SARA_CHARM_SCREEN_SCREEN_H
#define SARA_CHARM_SCREEN_SCREEN_H


#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "st7789_driver.h"


typedef struct screen_task {
    TaskHandle_t _handle;
    st7789_control_t _lcd;
} screen_task_t;

BaseType_t screen_task_start(screen_task_t* self);
void screen_task_stop(screen_task_t* self);


#endif //!SARA_CHARM_SCREEN_SCREEN_H
