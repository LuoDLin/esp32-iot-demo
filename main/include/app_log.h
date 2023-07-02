#ifndef _APP_LOG_H_
#define _APP_LOG_H_

#include <esp_log.h>
#define printi(fmt, ...) ESP_LOGI(__FILE__, fmt, ##__VA_ARGS__)
#define printe(fmt, ...) ESP_LOGE(__FILE__, fmt, ##__VA_ARGS__)

#endif
