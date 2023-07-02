#ifndef _APP_MQTT_H_
#define _APP_MQTT_H_

#include "app_log.h"
#include "app_err.h"

#define MQTT_CLIENT_URI "mqtt://broker.emqx.io:1883"

esp_err_t app_mqtt_client_init(void);


#endif
