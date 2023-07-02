#include "app_nvs.h"
#include "nvs_flash.h"

esp_err_t app_nvs_init(void){
    esp_err_t rtn = nvs_flash_init();
    if (rtn == ESP_ERR_NVS_NO_FREE_PAGES || rtn == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        rtn = nvs_flash_erase(); if( rtn != ESP_OK ) return rtn;
        rtn = nvs_flash_init(); if( rtn != ESP_OK ) return rtn;
    }
    return rtn;
}


