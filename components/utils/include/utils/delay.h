#ifndef SARA_CHARM_UTILITY_DELAY_H
#define SARA_CHARM_UTILITY_DELAY_H


#include <freertos/FreeRTOS.h>
#include <freertos/task.h>


inline void delay_ms(uint16_t num_ms);

inline void delay_ms(const uint16_t num_ms)
{
    vTaskDelay(pdMS_TO_TICKS(num_ms));
}


#endif //!SARA_CHARM_UTILITY_DELAY_H
