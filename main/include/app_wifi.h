#ifndef _APP_WIFI_H_
#define _APP_WIFI_H_

#include "app_err.h"

typedef void (*event_handler)(void* event_handler_arg,const char* event_base,int32_t event_id,void* event_data);
//WiFi初始化
esp_err_t app_wifi_init(void);
//WiFi启动
esp_err_t app_wifi_start(void);
//WiFi开始扫描
esp_err_t app_wifi_scan_start(void);
//WiFi设置账号密码
esp_err_t app_wifi_set_config(char *ssid, char *pswd);
//WiFi连接
esp_err_t app_wifi_connect();
//WiFi断开连接
esp_err_t app_wifi_disconnect(void);
//WiFi重连接
esp_err_t app_wifi_reconnect(void);


#endif