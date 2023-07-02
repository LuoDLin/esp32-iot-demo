#ifndef _APP_ERR_H_
#define _APP_ERR_H_

#include "esp_err.h"

#define APP_ERROR_RETURN(x) do {                                         \
        esp_err_t err_rc_ = (x);                                        \
        if ((err_rc_ != ESP_OK)) {                                      \
           return err_rc_;                                               \
        }                                                               \
    } while(0)



#endif
