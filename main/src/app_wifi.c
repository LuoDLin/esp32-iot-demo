#include "app_wifi.h"
#include "app_err.h"
#include "app_log.h"
#include <string.h>
#include <esp_wifi.h>
#include "app_provision.h"

#define DEFAULT_RECONNECT_TIME 15

esp_netif_t *netif = NULL;
static uint8_t reconnect_count = 0;
static TaskHandle_t app_wifi_task_handle = NULL;

esp_err_t app_wifi_set_config(char *ssid, char *pswd)
{
    wifi_config_t wifi_config;

    bzero(&wifi_config, sizeof(wifi_config_t));
    memcpy(wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    memcpy(wifi_config.sta.password, pswd, sizeof(wifi_config.sta.password));
    APP_ERROR_RETURN(esp_wifi_disconnect());
    APP_ERROR_RETURN(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    return ESP_OK;
}
// WiFi重连
esp_err_t app_wifi_reconnect(void)
{
    reconnect_count--;
    APP_ERROR_RETURN(esp_wifi_connect());
    return ESP_OK;
}
// WiFi连接
esp_err_t app_wifi_connect()
{
    reconnect_count = DEFAULT_RECONNECT_TIME;
    APP_ERROR_RETURN(app_wifi_reconnect());
    return ESP_OK;
}
// WiFi断开
esp_err_t app_wifi_disconnect(void)
{
    reconnect_count = 0;
    return esp_wifi_disconnect();
}

static void app_wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *data)
{
    printi("%s:%ld", event_base, event_id);
    if (event_base == WIFI_EVENT)
    {
        if (event_id == WIFI_EVENT_STA_START)
        {
            wifi_config_t config;
            ESP_ERROR_CHECK(esp_wifi_get_config(WIFI_IF_STA, &config)); // 获取WiFi配置
            if (strlen((char *)config.sta.ssid))
            { // 若有WiFi信息则连接
                printi("WiFi:-ssid:%s", config.sta.ssid);
                printi("WiFi:-pswd:%s", config.sta.password);
                app_wifi_connect();
            }
            else
            {
                printi("WiFi配置为空");
                app_smartconfig_start();
            }
        }
        else if (event_id == WIFI_EVENT_STA_DISCONNECTED)
        {
            wifi_event_sta_disconnected_t *event_data = data;
            printe("WiFi sta 断开连接");
            printe("WiFi:ssid:%.*s", event_data->ssid_len, event_data->ssid);
            // 204:握手超时      ---  信号低或密码错误
            // 15 :四次握手超时。 ---  密码错误
            // 201:无法扫描到目标AP  或信号低  有可能没有输入密码或未加密WiFi输入了密码
            printe("WiFi:reason:%d", event_data->reason);
            printe("WiFi:rssi:%d", event_data->rssi);
            if (event_data->reason == WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT)
            {
                printe("WiFi:密码错误");
            }
            else
            {
                if (reconnect_count)
                {
                    printe("WiFi 重新连接");
                    ESP_ERROR_CHECK(app_wifi_reconnect());
                }
                else
                {
                    printe("WiFi 重连接次数达最大");
                }
            }
        }
    }
}
static void app_wifi_task(void *arg)
{
    while (1)
    {
        xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);
    }
}


esp_err_t app_wifi_init(void)
{
    
    APP_ERROR_RETURN(esp_netif_init());
    netif = esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    APP_ERROR_RETURN(esp_wifi_init(&cfg));
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &app_wifi_event_handler, NULL);
    xTaskCreate(app_wifi_task, "wifi_task", MQTT_TASK_STACK_SIZE, NULL, 1, &app_wifi_task_handle);  //创建MQTT任务



    return ESP_OK;
}

// WiFi启动----在所有初始化完成以后启动
esp_err_t app_wifi_start(void)
{
    APP_ERROR_RETURN(esp_wifi_set_mode(WIFI_MODE_STA));
    APP_ERROR_RETURN(esp_wifi_start());
    return ESP_OK;
}
