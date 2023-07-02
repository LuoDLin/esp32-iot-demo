#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event.h"
#include "app_nvs.h"
#include "app_wifi.h"
#include "app_button.h"
#include "app_mqtt.h"


void app_main(void){
    ESP_ERROR_CHECK(app_nvs_init());                    //初始化内存
    ESP_ERROR_CHECK(esp_event_loop_create_default());   //初始化事件组
    app_button_init();                                  //按钮初始化
    ESP_ERROR_CHECK(app_wifi_init());                   //WiFi初始化
    ESP_ERROR_CHECK(app_mqtt_client_init());            //mqtt客户端初始化
    ESP_ERROR_CHECK(app_wifi_start());
    vTaskDelete(NULL);
}

