#ifndef SARA_CHARM_UTILS_ESP_TRY_H
#define SARA_CHARM_UTILS_ESP_TRY_H


#include <stdbool.h>

#include <esp_err.h>


#define ESP_TRY(expr)               \
do {                                \
    esp_err_t esp_try_res = (expr); \
    if (esp_try_res != ESP_OK)      \
    {                               \
        return esp_try_res;         \
    }                               \
} while (false)


#endif //!SARA_CHARM_UTILS_ESP_TRY_H
