#include "app_provision.h"
#include "esp_event.h"
#include "esp_smartconfig.h"
#include "app_wifi.h"



static void app_provision_event_handler(void *arg, esp_event_base_t event_base,int32_t event_id, void *event_data){
    printi("%s:%ld", event_base, event_id);
    if (event_base == SC_EVENT){
        if ( event_id == SC_EVENT_GOT_SSID_PSWD){
            printi("Got SSID and password");
            smartconfig_event_got_ssid_pswd_t *evt = (smartconfig_event_got_ssid_pswd_t *)event_data;
            uint8_t rvd_data[33] = {0};
            app_wifi_set_config((char*)evt->ssid,(char*)evt->password);
            ESP_ERROR_CHECK(app_wifi_connect());
            if (evt->type == SC_TYPE_ESPTOUCH_V2){
                ESP_ERROR_CHECK(esp_smartconfig_get_rvd_data(rvd_data, sizeof(rvd_data)));
                printi("RVD_DATA:%s",rvd_data);
            }
        }
        else if ( event_id == SC_EVENT_SEND_ACK_DONE )
        {
            printi("Send ACK Done");
            ESP_ERROR_CHECK(app_smartconfig_stop());
        }
    }
}
esp_err_t app_smartconfig_start(void)
{
    printi("Smart Config Start!");
    APP_ERROR_RETURN(esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &app_provision_event_handler, NULL));
    APP_ERROR_RETURN(esp_smartconfig_set_type(SC_TYPE_ESPTOUCH_V2));
    smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
    APP_ERROR_RETURN(esp_smartconfig_start(&cfg));
    return ESP_OK;
}
esp_err_t app_smartconfig_stop(void)
{
    printi("Smart Config Stop! ");
    APP_ERROR_RETURN(esp_event_handler_unregister(SC_EVENT,ESP_EVENT_ANY_ID,&app_provision_event_handler));
    APP_ERROR_RETURN(esp_smartconfig_stop());
    return ESP_OK;
}