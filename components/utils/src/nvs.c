#include "utils/nvs.h"
#include "nvs.h"

#include <nvs_flash.h>


esp_err_t init_nvs(void)
{
    esp_err_t res = nvs_flash_init();
    if (res == ESP_ERR_NVS_NO_FREE_PAGES ||
        res == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        res = nvs_flash_erase();
        if (res != ESP_OK)
        {
            res = nvs_flash_init();
        }
    }

    return res;
}

void deinit_nvs()
{
    (void)nvs_flash_deinit();
}
